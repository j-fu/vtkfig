///
///   \example  example-custom-vtkfig.cxx
///
///   Custom vtk pipeline with vtkfig::Figure.
///
///  Example showing creation of custom scenes using  vtkfig::Frame and vtkfig::Figure base class.
///

#include <chrono>
#include "vtkStructuredGrid.h"
#include "vtkPointData.h"
#include "vtkFloatArray.h"
#include "vtkActor.h"
#include "vtkLine.h"
#include "vtkOutlineFilter.h"
#include "vtkStructuredGridGeometryFilter.h"

#include "vtkfigFrame.h"
#include "vtkfigFigure.h"
#include "vtkfigTools.h"


namespace vtkfig
{
  
  /// 
  /// Surface and contour plot of 2/3D scalar data
  ///
  class Scene2D: public Figure
  {
    class polyline
    {
    public:
      vtkSmartPointer<vtkPolyData> polydata;
      vtkSmartPointer<vtkPolyDataMapper> mapper;
      vtkSmartPointer<vtkActor> actor;
      polyline(vtkSmartPointer<vtkPoints> points,vtkSmartPointer<vtkCellArray> lines,vtkSmartPointer<vtkUnsignedCharArray> colors)
      {
        polydata=vtkSmartPointer<vtkPolyData>::New();
        polydata->SetPoints(points);
        polydata->SetLines(lines);
        //        polydata->GetCellData()->SetScalars(colors);
        mapper=vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputData(polydata);
        actor=vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
      }
    };
    std::vector<polyline> polylines;
    
  public:
    vtkSmartPointer<vtkPoints> build_points;
    vtkSmartPointer<vtkCellArray> build_lines;
    vtkSmartPointer<vtkUnsignedCharArray> build_colors;
    
    vtkIdType last_id;
    unsigned char current_rgb[3];
    void BeginLines(void)
    {
      build_points=vtkSmartPointer<vtkPoints>::New();
      build_lines=vtkSmartPointer<vtkCellArray>::New();
      build_colors=vtkSmartPointer<vtkUnsignedCharArray>::New();
      build_colors->SetNumberOfComponents(3);
      last_id=-1;
      current_rgb[0]=0;
      current_rgb[1]=0;
      current_rgb[2]=0;
    }
    void Color(double r, double g, double b)
    {
      current_rgb[0]=(unsigned char)(r*255.0);
      current_rgb[1]=(unsigned char)(g*255.0);     
      current_rgb[2]=(unsigned char)(b*255.0);
    }
    void Vertex(double x, double y)
    {
      auto this_id=build_points->InsertNextPoint(x,y,0.0);
      if (last_id>=0)
      {
        auto line=vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetId(0, last_id);
        line->GetPointIds()->SetId(1, this_id);
        build_lines->InsertNextCell(line);
        build_colors->InsertNextTypedTuple(current_rgb);
      }
    }
    void End(void)
    {
      polylines.emplace_back(build_points, build_lines, build_colors);
      auto npoly=polylines.size();
      this->AddActor(polylines[npoly-1].actor);
    }
    Scene2D(){};
    
    static std::shared_ptr<Scene2D> New() { return std::make_shared<Scene2D>();}
    
  };
  
}

void test(void)
{
  // !!! check if   https://vtk.org/Wiki/VTK/Examples/Cxx/GeometricObjects/ColoredLines works
  auto scene=vtkfig::Scene2D::New();
  scene->SetViewVolume(-1,1,-1,1,-1,1);
  auto frame=vtkfig::Frame::New();
  frame->AddFigure(scene);
  scene->BeginLines();
  for (double x=0;x<1.0; x+=0.1)
    scene->Vertex(x,sin(x));
  scene->End();
  frame->AddFigure(scene);
  frame->Interact();
}


inline double G(double x,double y, double t) 
{
  
  return exp(-(x*x+y*y))*sin(t+x)*cos(y-t);
}


int main(void)
{
  test();
  exit(1);
  size_t nspin=vtkfig::NSpin();

  cout.sync_with_stdio(true);
  
  const int Nx = 200;
  const int Ny = 250;
  
  std::vector<double> x(Nx);
  std::vector<double> y(Ny);
  std::vector<double> z(Nx*Ny);
  
  const double x_low = -2.5;
  const double x_upp = 1.5;
  const double y_low = -2.5;
  const double y_upp = 4;
  const double dx = (x_upp-x_low)/(Nx-1);
  const double dy = (y_upp-y_low)/(Ny-1);

  std::shared_ptr<vtkfig::Frame> frame=vtkfig::Frame::New();
    


  

  for (int i=0; i<Nx; i++)
    x[i] = x_low+i*dx;
  
  for (int i=0; i<Ny; i++)
    y[i] = y_low + i*dy;


  double t=0;
  double dt=0.1;
  size_t ii=0;
  auto t0=std::chrono::system_clock::now();
  double i0=ii;

  std::shared_ptr<vtkfig::Figure> figure=vtkfig::Figure::New();
  figure->SetViewVolume(0,1,0,1,0,1);
  int i,j,k;
  
  
  auto gridfunc=vtkSmartPointer<vtkStructuredGrid>::New();
  auto points=vtkSmartPointer<vtkPoints>::New();
  auto colors=vtkSmartPointer<vtkFloatArray>::New();
  
  gridfunc->SetDimensions(Nx, Ny, 1);
  
  points = vtkSmartPointer<vtkPoints>::New();
  for (j = 0; j < Ny; j++)
  {
    for (i = 0; i < Nx; i++)
    {
      points->InsertNextPoint(x[i], y[j], 0);
    }
  }
  gridfunc->SetPoints(points);
  
  colors = vtkSmartPointer<vtkFloatArray>::New();
  colors->SetNumberOfComponents(1);
  colors->SetNumberOfTuples(Nx*Ny);
  k = 0;
  for (j = 0; j < Ny; j++)
    for (i = 0; i < Nx; i++)
    {
      colors->InsertComponent(k, 0, 0);
      k++;
    }
  
  auto rgbcolors=vtkfig::RGBTable
    { 
      {0.0, 0.0, 0.0, 1.0},
      {0.5, 0.0, 1.0, 0.0},
      {1.0, 1.0, 0.0, 0.0}
    };

  vtkSmartPointer<vtkLookupTable> lut = vtkfig::internal::BuildLookupTable(rgbcolors,255);
  
  gridfunc->GetPointData()->SetScalars(colors);
  
  
  // filter to geometry primitive
  auto geometry = vtkSmartPointer<vtkStructuredGridGeometryFilter>::New();
  geometry->SetInputDataObject(gridfunc);
  
  
  // map gridfunction
  auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(geometry->GetOutputPort());
  
  
  // create plot surface actor
  auto surfplot = vtkSmartPointer<vtkActor>::New();
  surfplot->SetMapper(mapper);
  mapper->SetLookupTable(lut);
  mapper->UseLookupTableScalarRangeOn();
  
  
  // create outline
  auto outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
  outlinefilter->SetInputConnection(geometry->GetOutputPort());
  
  auto outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  outlineMapper->SetInputConnection(outlinefilter->GetOutputPort());
  auto outline = vtkSmartPointer<vtkActor>::New();
  outline->SetMapper(outlineMapper);
  outline->GetProperty()->SetColor(0, 0, 0);
  
  
  

  // renderer
  figure->AddActor(surfplot);
  figure->AddActor(outline);
  figure->AddActor2D(vtkfig::internal::BuildColorBar(mapper));
  frame->AddFigure(figure);
  
  
  
  
  while (ii <nspin)
  {
    
    for (int i=0; i<Nx; i++)
      for (int j=0; j<Ny; j++)
        z[j*Nx+i] = G(x[i],y[j],t);
    
    float vmax=-10000;
    float vmin=10000;
    for (int j = 0; j < Ny; j++)
    {
      for (int i = 0; i < Nx; i++)
      {
        int k=j*Nx+i;
        float v=z[k];
        vmin=std::min(v,vmin);
        vmax=std::max(v,vmax);
        double  p[3];
        points->GetPoint(k,p);
        p[2]=v;
        points->SetPoint(k,p);
        colors->InsertComponent(k, 0,v);
      }
    }
    
    points->Modified();
    colors->Modified();
    gridfunc->Modified();

    lut->SetTableRange(vmin,vmax);
    lut->Modified();
    
    frame->Show();
    

    t+=dt;
    auto t1=std::chrono::system_clock::now();
    double dt=std::chrono::duration_cast<std::chrono::duration<double>>(t1-t0).count();
    double i1=ii;
    if (dt>4.0)
    {
      printf("Frame rate: %.2f fps\n",(double)(i1-i0)/4.0);
      t0=std::chrono::system_clock::now();
      i0=ii;
      fflush(stdout);
    }
    ii++;
  }

}
