#include "vtkfigFrame.h"
#include "vtkfigFigure.h"


#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkProperty.h>
#include <vtkIdList.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkGeometryFilter.h>
#include <vtkContourFilter.h>
#include <vtkUnstructuredGridGeometryFilter.h>


#include <vtkDelaunay2D.h>
#include <vtkMath.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

inline double G(double x,double y, double t) 
{
  
  return exp(-(x*x+y*y))*sin(t+3.*x)*cos(4.*y-t);
}


  ///////////////////////////////////////////
class TriContour2D: public vtkfig::Figure
  {
  public:
  
  TriContour2D();
  
  // template<typename V>
  //     void Add(const V &xcoord, 
  //              const V &ycoord, 
  //              const V &values);

      
      void ShowSurface(bool b) {show_surface=b;}
      void ShowContour(bool b) {show_contour=b;}
      void ShowSurfaceColorbar(bool b) {show_surface_colorbar=b;}
      void ShowContourColorbar(bool b) {show_contour_colorbar=b;}

      void SetSurfaceRGBTable(RGBTable & tab, int tabsize)
      {
        surface_lut=BuildLookupTable(tab,tabsize);
      }
      void SetContourRGBTable(RGBTable & tab, int tabsize)
      {
        contour_lut=BuildLookupTable(tab,tabsize);
      }

    void Add(vtkSmartPointer<vtkUnstructuredGrid> gridfunc);
      
    private:

      vtkSmartPointer<vtkLookupTable> surface_lut;
      vtkSmartPointer<vtkLookupTable> contour_lut;

      bool show_surface=true;
      bool show_contour=true;
      bool show_surface_colorbar=true;
      bool show_contour_colorbar=false;
  };


TriContour2D::TriContour2D(): vtkfig::Figure()
  {
    RGBTable surface_rgb={{0,0,0,1},{1,1,0,0}};
    RGBTable contour_rgb={{0,0,0,0},{1,0,0,0}};
    surface_lut=BuildLookupTable(surface_rgb,255);
    contour_lut=BuildLookupTable(contour_rgb,2);
  }


void TriContour2D::Add(vtkSmartPointer<vtkUnstructuredGrid> gridfunc)
{
    // filter to geometry primitive

  // see http://www.vtk.org/Wiki/VTK/Examples/Cxx/PolyData/GeometryFilter

    vtkSmartPointer<vtkGeometryFilter> geometry =  vtkSmartPointer<vtkGeometryFilter>::New();
    geometry->SetInputDataObject(gridfunc);
    double vrange[2];
    gridfunc->GetScalarRange(vrange);
    cout << vrange[0] << " " << vrange[1] << endl;
    int lines=10;

    if (show_surface)
    {
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(geometry->GetOutputPort());
      mapper->SetScalarRange(vrange[0], vrange[1]);
      mapper->SetLookupTable(surface_lut);
      mapper->Update();

      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->SetMapper(mapper);
      Figure::AddActor(plot);
      
      if (show_surface_colorbar)
        Figure::AddActor(Figure::BuildColorBar(mapper));
    }


    if (show_contour)
    {
      vtkSmartPointer<vtkContourFilter> isocontours = vtkSmartPointer<vtkContourFilter>::New();
      isocontours->SetInputConnection(geometry->GetOutputPort());
      double tempdiff = (vrange[1]-vrange[0])/(10*lines);
      isocontours->GenerateValues(lines, vrange[0]+tempdiff, vrange[1]-tempdiff);


      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(isocontours->GetOutputPort());
      mapper->SetScalarRange(vrange[0], vrange[1]);
      mapper->SetLookupTable(contour_lut);

      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->SetMapper(mapper);
      Figure::AddActor(plot);
      if (show_contour_colorbar)
        Figure::AddActor(Figure::BuildColorBar(mapper));

    }

  }


int main(void)
{
  
  
  // Generate a 10 x 10 grid of points
  vtkSmartPointer<vtkPoints> inpoints =
    vtkSmartPointer<vtkPoints>::New();
  for(double x = 0; x < 1; x+=0.02)
  {
    for(double y = 0; y < 1; y+=0.02)
    {
      inpoints->InsertNextPoint(x + vtkMath::Random(-.025, .025),
                              y+ vtkMath::Random(-.025,.025),
                              0);
    }
  }
  
  vtkSmartPointer<vtkPolyData> aPolyData =
    vtkSmartPointer<vtkPolyData>::New();
  aPolyData->SetPoints(inpoints);
 

  vtkSmartPointer<vtkDelaunay2D> delaunay =
    vtkSmartPointer<vtkDelaunay2D>::New();
  delaunay->SetInputData(aPolyData);

  vtkSmartPointer<vtkPolyData>dgrid=delaunay->GetOutput();
  delaunay->Update();


  
  cout << dgrid->GetNumberOfPolys() << endl;
  

  auto npoints=dgrid->GetNumberOfPoints();
  auto ncells=dgrid->GetNumberOfPolys();

  std::vector<double>points;
  std::vector<double>values;
  std::vector<int>cells;



  vtkSmartPointer<vtkPoints> vtkpoints=dgrid->GetPoints();
  for (int i=0;i<npoints;i++)
  {
    double x[3];
    vtkpoints->GetPoint(i,x);
    points.push_back(x[0]);
    points.push_back(x[1]);
  }

 vtkSmartPointer<vtkIdList> pts =
   vtkSmartPointer<vtkIdList>::New();

  for (vtkIdType i=0;i<ncells;i++)
  {
    int c[3];

    dgrid->GetCellPoints(i,pts);
    c[0]=pts->GetId(0);
    c[1]=pts->GetId(1);
    c[2]=pts->GetId(2);

    cells.push_back(c[0]);
    cells.push_back(c[1]);
    cells.push_back(c[2]);
//    cout << c[0] << "  "  << c[1] << "  " << c[2] << endl;
  }
  cout << cells.size()/3 << " "  << ncells << endl;


  for (int ipoint=0;ipoint<points.size(); ipoint+=2)
  {
    values.push_back( G(points[ipoint+0],points[ipoint+1],5.0));
  }
  

  cout << values.size() << " " << points.size()/2 << endl;
////////////////////////////////////////////////////////////////////77
  vtkSmartPointer<vtkUnstructuredGrid> grid=vtkSmartPointer<vtkUnstructuredGrid>::New();
  
  for (int icell=0;icell<cells.size(); icell+=3)
  {
    vtkIdType 	 c[3]={cells[icell+0],cells[icell+1],cells[icell+2]};
    grid->InsertNextCell(VTK_TRIANGLE,3,c);
  }

  vtkSmartPointer<vtkPoints> gridpoints = vtkSmartPointer<vtkPoints>::New();
  for (int ipoint=0;ipoint<points.size(); ipoint+=2)
  {
    gridpoints->InsertNextPoint(points[ipoint+0],points[ipoint+1],0);
  }
  grid->SetPoints(gridpoints);


  vtkSmartPointer<vtkFloatArray> gridvalues = vtkSmartPointer<vtkFloatArray>::New();
  gridvalues->SetNumberOfComponents(1);
  gridvalues->SetNumberOfTuples(npoints);
  for (int i=0;i<npoints; i++)
  {
    gridvalues->InsertComponent(i,0,values[i]);
  }

  grid->GetPointData()->SetScalars(gridvalues);


  auto frame=vtkfig::Frame();
  auto colors=vtkfig::Figure::RGBTable
    { 
      {0.0, 0.0, 0.0, 1.0},
      {0.5, 0.0, 1.0, 0.0},
      {1.0, 1.0, 0.0, 0.0}
    };
    auto contour=TriContour2D();
    contour.SetSurfaceRGBTable(colors,255);
    contour.Add(grid);
    frame.Clear();
    frame.Add(contour);
    frame.Show();
    frame.Interact();

}
