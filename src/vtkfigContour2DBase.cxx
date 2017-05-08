
#include "vtkSliderRepresentation2D.h"
#include "vtkProperty2D.h"
#include "vtkTextProperty.h"
#include "vtkfigContour2DBase.h"

namespace vtkfig
{

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
