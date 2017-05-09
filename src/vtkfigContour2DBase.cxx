
#include "vtkSliderRepresentation2D.h"
#include "vtkProperty2D.h"
#include "vtkTextProperty.h"
#include "vtkfigContour2DBase.h"

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
    Contour2DBase *contour2d;
  };



  void Contour2DBase::RTSetInteractor(vtkSmartPointer<vtkRenderWindowInteractor> interactor) 
  {
    if (show_slider && show_contour)
    {
      sliderWidget->SetInteractor(interactor);
      sliderWidget->EnabledOn();
    }
  };

  void Contour2DBase::AddSlider()
  {
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
      }

  }

}