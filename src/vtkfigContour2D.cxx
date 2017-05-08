#include "vtkRectilinearGridGeometryFilter.h"
#include "vtkContourFilter.h"
#include "vtkOutlineFilter.h"
#include "vtkPointData.h"
#include "vtkActor.h"
#include "vtkCommand.h"
#include "vtkProperty2D.h"
#include "vtkTextProperty.h"
#include "vtkSliderRepresentation2D.h"
#include "vtkfigContour2D.h"


namespace vtkfig
{



////////////////////////////////////////////////////////////
  Contour2D::Contour2D(): Contour2DBase()
  {
  }
  
  
  void Contour2D::RTBuild()
  {

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


    if (show_surface)
    {
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(geometry->GetOutputPort());

      mapper->UseLookupTableScalarRangeOn();
      mapper->SetLookupTable(surface_lut);
      mapper->ImmediateModeRenderingOn();

      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->SetMapper(mapper);
      Figure::RTAddActor(plot);
      
      if (show_surface_colorbar)
        Figure::RTAddActor2D(BuildColorBar(mapper));
    }


    if (show_contour)
    {

      isocontours->SetInputConnection(geometry->GetOutputPort());

      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(isocontours->GetOutputPort());
      mapper->UseLookupTableScalarRangeOn();
      mapper->SetLookupTable(contour_lut);

      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->SetMapper(mapper);
      Figure::RTAddActor(plot);
      if (show_contour_colorbar)
        Figure::RTAddActor2D(BuildColorBar(mapper));
      
      AddSlider();

    }

  }

}
