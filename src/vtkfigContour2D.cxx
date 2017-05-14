#include "vtkRectilinearGridGeometryFilter.h"
#include "vtkStructuredGridGeometryFilter.h"
#include "vtkStructuredGrid.h"
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
  
  
  void Contour2D::RTBuild(
        vtkSmartPointer<vtkRenderWindow> window,
        vtkSmartPointer<vtkRenderWindowInteractor> interactor,
        vtkSmartPointer<vtkRenderer> renderer)
  {
    bool do_warp=false;

    vtkSmartPointer<vtkRectilinearGrid> gridfunc=vtkSmartPointer<vtkRectilinearGrid>::New();
    int Nx = xcoord->GetNumberOfTuples();
    int Ny = ycoord->GetNumberOfTuples();

    // Create rectilinear grid
    gridfunc->SetDimensions(Nx, Ny, 1);
    gridfunc->SetXCoordinates(xcoord);
    gridfunc->SetYCoordinates(ycoord);
    gridfunc->GetPointData()->SetScalars(values);

    // filter to geometry primitive
    vtkSmartPointer<vtkRectilinearGridGeometryFilter> geometry =  vtkSmartPointer<vtkRectilinearGridGeometryFilter>::New();
    geometry->SetInputDataObject(gridfunc);

    if (0&&show_elevation)
    {
      vtkSmartPointer<vtkStructuredGrid> sgridfunc= vtkSmartPointer<vtkStructuredGrid>::New();
      sgridfunc->SetDimensions(Nx, Ny, 1);
      
      vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
      for (int k=0, j = 0; j < Ny; j++)
      {
        double *y=ycoord->GetTuple(j);
        for (int i = 0; i < Nx; i++,k++)
        {
          double *x=xcoord->GetTuple(i);
          double *v=values->GetTuple(k);
          points->InsertNextPoint(*x,*y,*v);

        }
      }
      sgridfunc->SetPoints(points);
      
      vtkSmartPointer<vtkStructuredGridGeometryFilter>geometry =
        vtkSmartPointer<vtkStructuredGridGeometryFilter>::New();
      geometry->SetInputDataObject(sgridfunc);

             
      
      // map gridfunction
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(geometry->GetOutputPort());
      
      
      // create plot surface actor
      vtkSmartPointer<vtkActor> surfplot = vtkSmartPointer<vtkActor>::New();
      surfplot->SetMapper(mapper);
      surfplot->GetProperty()->SetColor(0.5,0.5,0.5);


      // mapper->SetLookupTable(lut);
      // mapper->UseLookupTableScalarRangeOn();
      
      
      // create outline
      vtkSmartPointer<vtkOutlineFilter> outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
      outlinefilter->SetInputConnection(geometry->GetOutputPort());
      
      vtkSmartPointer<vtkPolyDataMapper>outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      outlineMapper->SetInputConnection(outlinefilter->GetOutputPort());
      vtkSmartPointer<vtkActor> outline = vtkSmartPointer<vtkActor>::New();
      outline->SetMapper(outlineMapper);
      outline->GetProperty()->SetColor(0, 0, 0);
      Figure::RTAddActor(surfplot);
      Figure::RTAddActor(outline);

    }

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
      
      if (show_slider)
        AddSlider(interactor,renderer);

    }

  }

}
