#include "vtkRectilinearGridGeometryFilter.h"
#include "vtkContourFilter.h"
#include "vtkOutlineFilter.h"
#include "vtkPointData.h"
#include "vtkActor.h"


#include "vtkfigContour2D.h"

namespace vtkfig
{

////////////////////////////////////////////////////////////
  Contour2D::Contour2D(): Figure()
  {
    RGBTable surface_rgb={{0,0,0,1},{1,1,0,0}};
    RGBTable contour_rgb={{0,0,0,0},{1,0,0,0}};
    surface_lut=BuildLookupTable(surface_rgb,255);
    contour_lut=BuildLookupTable(contour_rgb,2);
  }
  
  
  void Contour2D::Add(vtkSmartPointer<vtkFloatArray> xcoord ,vtkSmartPointer<vtkFloatArray> ycoord ,vtkSmartPointer<vtkFloatArray> values )
  {
    
    if (!Figure::IsEmpty())
      throw std::runtime_error("Contor2D already has data");


    vtkSmartPointer<vtkRectilinearGrid> gridfunc=vtkSmartPointer<vtkRectilinearGrid>::New();
    int Nx = xcoord->GetNumberOfTuples();
    int Ny = ycoord->GetNumberOfTuples();
    int lines=10;

    // Create rectilinear grid
    gridfunc->SetDimensions(Nx, Ny, 1);
    gridfunc->SetXCoordinates(xcoord);
    gridfunc->SetYCoordinates(ycoord);
    gridfunc->GetPointData()->SetScalars(values);

    // filter to geometry primitive
    vtkSmartPointer<vtkRectilinearGridGeometryFilter> geometry =  vtkSmartPointer<vtkRectilinearGridGeometryFilter>::New();
    geometry->SetInputDataObject(gridfunc);
    double vrange[2];
    gridfunc->GetScalarRange(vrange);


    if (show_surface)
    {
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(geometry->GetOutputPort());
      mapper->SetScalarRange(vrange[0], vrange[1]);
      mapper->SetLookupTable(surface_lut);
      
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

}
