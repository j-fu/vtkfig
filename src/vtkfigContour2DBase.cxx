
#include "vtkSliderRepresentation2D.h"
#include "vtkProperty2D.h"
#include "vtkTextProperty.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"

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
      vtkSliderWidget *sliderWidget =         reinterpret_cast<vtkSliderWidget*>(caller);
      this->contour2d->state.ncont=static_cast<vtkSliderRepresentation *>(sliderWidget->GetRepresentation())->GetValue();
      double tempdiff = (this->contour2d->state.real_vmax-this->contour2d->state.real_vmin)/(10*this->contour2d->state.ncont);
      this->contour2d->isocontours->GenerateValues(this->contour2d->state.ncont, this->contour2d->state.real_vmin+tempdiff, this->contour2d->state.real_vmax-tempdiff);
      this->contour2d->isocontours->Modified();
    }
    mySliderCallback():contour2d(0) {}
    Contour2DBase *contour2d;
  };



  void Contour2DBase::AddSlider(vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                                vtkSmartPointer<vtkRenderer> renderer)
  {
      if (state.show_slider)
      {

        auto sliderRep = vtkSmartPointer<vtkSliderRepresentation2D>::New();
        
        
        sliderRep->SetMinimumValue(0.0);
        sliderRep->SetMaximumValue(20.0);
        sliderRep->SetLabelFormat("%.0f");
        sliderRep->SetValue(state.ncont);
        
        sliderRep->SetTitleText("Number of Isolines");
        sliderRep->GetSliderProperty()->SetColor(0.5,0.5,0.5);
        sliderRep->GetTitleProperty()->SetColor(0.5,0.5,0.5);
        sliderRep->GetLabelProperty()->SetColor(0.5,0.5,0.5);
        sliderRep->GetSelectedProperty()->SetColor(0,0,0);
        sliderRep->GetTubeProperty()->SetColor(0.5,0.5,0.5);
        sliderRep->GetCapProperty()->SetColor(0.5,0.5,0.5);
        
        
        sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedViewport();
        sliderRep->GetPoint1Coordinate()->SetValue(0.2,0.1);
        sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedViewport();
        sliderRep->GetPoint2Coordinate()->SetValue(0.8,0.1);
        
        sliderRep->SetSliderLength(0.02);
        sliderRep->SetSliderWidth(0.02);
        sliderRep->SetEndCapLength(0.01);
        
        sliderWidget->SetRepresentation(sliderRep);
        
        auto callback =   vtkSmartPointer<mySliderCallback>::New();
        callback->contour2d = this;
        sliderWidget->AddObserver(vtkCommand::InteractionEvent,callback);
        sliderWidget->SetDefaultRenderer(renderer);
        sliderWidget->SetCurrentRenderer(renderer);
        sliderWidget->SetInteractor(interactor);
        sliderWidget->SetAnimationModeToAnimate();
        sliderWidget->EnabledOn();
      }


  }

  void Contour2DBase::ProcessData(
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer,
    vtkSmartPointer<vtkPolyDataAlgorithm> data, 
    double bounds[6])
  {

    // transform everything to [0,1]x[0,1]
    // 
    auto transform =  vtkSmartPointer<vtkTransform>::New();
    double xsize=bounds[1]-bounds[0];
    double ysize=bounds[3]-bounds[2];
    
    double xysize=std::max(xsize,ysize);
    
    if (state.keep_aspect)
    {
      if (xsize>ysize)
        transform->Translate(0,0.5*(xsize-ysize)/xysize,0);
      else
        transform->Translate(0.5*(ysize-xsize)/xysize,0,0);
      
      transform->Scale(1.0/xysize, 1.0/xysize,1);
    }
    else
    {
      if (state.aspect>1.0)
      {
        transform->Translate(0,0.5-0.5/state.aspect,0);
        transform->Scale(1.0/xsize, 1.0/(state.aspect*ysize),1);
      }
      else
      {
        transform->Translate(0.5-0.5*state.aspect,0,0);
        transform->Scale(state.aspect/xsize, 1.0/ysize,1);
      }
    }
    
    transform->Translate(-bounds[0],-bounds[2],0);
    
    auto transformFilter =vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformFilter->SetInputConnection(data->GetOutputPort());
    transformFilter->SetTransform(transform);
    transformFilter->Update();
    
    
    if (state.show_surface)
    {
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(transformFilter->GetOutputPort());
      
      mapper->UseLookupTableScalarRangeOn();
      mapper->SetLookupTable(surface_lut);
      mapper->ImmediateModeRenderingOn();
      
      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->SetMapper(mapper);
      Figure::RTAddActor(plot);
      
      if (state.show_surface_colorbar)
        Figure::RTAddActor2D(BuildColorBar(mapper));
    }
    
    
    if (state.show_contour)
    {
      
      isocontours->SetInputConnection(transformFilter->GetOutputPort());
      
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(isocontours->GetOutputPort());
      mapper->UseLookupTableScalarRangeOn();
      mapper->SetLookupTable(contour_lut);
      
      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->SetMapper(mapper);
      Figure::RTAddActor(plot);
      if (state.show_contour_colorbar)
        Figure::RTAddActor2D(BuildColorBar(mapper));
      
      if (state.show_slider)
        AddSlider(interactor,renderer);
      
    } 
  }
}
