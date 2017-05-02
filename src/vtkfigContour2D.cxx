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


class mySliderCallback : public vtkCommand
{
public:
  static mySliderCallback *New() 
    {
    return new mySliderCallback;
    }
  virtual void Execute(vtkObject *caller, unsigned long, void*)
    {
    vtkSliderWidget *sliderWidget = 
      reinterpret_cast<vtkSliderWidget*>(caller);
    this->contour2d->ncont=static_cast<vtkSliderRepresentation *>(sliderWidget->GetRepresentation())->GetValue();
    double tempdiff = (this->contour2d->vmax-this->contour2d->vmin)/(10*this->contour2d->ncont);
    this->contour2d->isocontours->GenerateValues(this->contour2d->ncont, this->contour2d->vmin+tempdiff, this->contour2d->vmax-tempdiff);
    this->contour2d->isocontours->Modified();
    }
  mySliderCallback():contour2d(0) {}
  Contour2D *contour2d;
};

////////////////////////////////////////////////////////////
  Contour2D::Contour2D(): Figure()
  {
    RGBTable surface_rgb={{0,0,0,1},{1,1,0,0}};
    RGBTable contour_rgb={{0,0,0,0},{1,0,0,0}};
    surface_lut=BuildLookupTable(surface_rgb,255);
    contour_lut=BuildLookupTable(contour_rgb,2);
    isocontours = vtkSmartPointer<vtkContourFilter>::New();
    sliderWidget = vtkSmartPointer<vtkSliderWidget>::New();
  }
  
  void Contour2D::SetInteractor(vtkSmartPointer<vtkRenderWindowInteractor> interactor) 
  {
    if (show_slider && show_contour)
    {
      sliderWidget->SetInteractor(interactor);
      sliderWidget->EnabledOn();
    }
  };
  
  void Contour2D::Build()
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
      Figure::AddActor(plot);
      
      if (show_surface_colorbar)
        Figure::AddActor2D(BuildColorBar(mapper));
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
      Figure::AddActor(plot);
      if (show_contour_colorbar)
        Figure::AddActor2D(BuildColorBar(mapper));
      
      
      if (show_slider)
      {
        vtkSmartPointer<vtkSliderRepresentation2D> sliderRep =
          vtkSmartPointer<vtkSliderRepresentation2D>::New();
        
        sliderRep->SetMinimumValue(0.0);
        sliderRep->SetMaximumValue(20.0);
        sliderRep->SetLabelFormat("%.0f");
        sliderRep->SetValue(ncont);
        
        sliderRep->SetTitleText("Number of Isolines");
        sliderRep->GetSliderProperty()->SetColor(0.5,0.5,0.5);
        sliderRep->GetTitleProperty()->SetColor(0.5,0.5,0.5);
        sliderRep->GetLabelProperty()->SetColor(0.5,0.5,0.5);
        sliderRep->GetSelectedProperty()->SetColor(0,0,0);
        sliderRep->GetTubeProperty()->SetColor(0.5,0.5,0.5);
        sliderRep->GetCapProperty()->SetColor(0.5,0.5,0.5);
        
        
        sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedViewport();
        sliderRep->GetPoint1Coordinate()->SetValue(0.2,0.9);
        sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedViewport();
        sliderRep->GetPoint2Coordinate()->SetValue(0.8,0.9);
        
        sliderRep->SetSliderLength(0.02);
        sliderRep->SetSliderWidth(0.02);
        sliderRep->SetEndCapLength(0.01);
        
        sliderWidget->SetRepresentation(sliderRep);
        sliderWidget->SetAnimationModeToAnimate();
        
        vtkSmartPointer<mySliderCallback> callback =
          vtkSmartPointer<mySliderCallback>::New();
        callback->contour2d = this;
        sliderWidget->AddObserver(vtkCommand::InteractionEvent,callback);
        

        sliderWidget->AddObserver(vtkCommand::InteractionEvent,callback);
      }
    }

  }

}
