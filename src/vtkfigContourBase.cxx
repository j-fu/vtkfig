#include "vtkSliderRepresentation2D.h"
#include "vtkProperty2D.h"
#include "vtkAlgorithmOutput.h"
#include "vtkTextProperty.h"
#include "vtkTransform.h"
#include "vtkTransformFilter.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkRectilinearGrid.h"
#include "vtkGeometryFilter.h"
#include "vtkRectilinearGridGeometryFilter.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridGeometryFilter.h"
#include "vtkPlane.h"
#include "vtkCutter.h"
#include "vtkOutlineFilter.h"



#include "vtkfigContourBase.h"


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
      this->contour2d->state.num_contours=static_cast<vtkSliderRepresentation *>(sliderWidget->GetRepresentation())->GetValue();
      
      this->contour2d->SetVMinMax(this->contour2d->state.real_vmin, this->contour2d->state.real_vmax);
    }
    mySliderCallback():contour2d(0) {}
    ContourBase *contour2d;
  };



  void ContourBase::AddSlider(vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                                vtkSmartPointer<vtkRenderer> renderer)
  {
      if (state.show_slider)
      {

        auto sliderRep = vtkSmartPointer<vtkSliderRepresentation2D>::New();
        
        
        sliderRep->SetMinimumValue(0.0);
        sliderRep->SetMaximumValue(state.max_num_contours);
        sliderRep->SetLabelFormat("%.0f");
        sliderRep->SetValue(state.num_contours);
        
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

  template <class DATA, class FILTER>
  void ContourBase::RTBuild2D(
    vtkSmartPointer<vtkRenderWindow> window,
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer,
    vtkSmartPointer<DATA> gridfunc)
  {
    double bounds[6];
    gridfunc->GetBounds(bounds);

    auto geometry=vtkSmartPointer<FILTER>::New();
    geometry->SetInputDataObject(gridfunc);

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
    
    auto transformFilter =vtkSmartPointer<vtkTransformFilter>::New();
    transformFilter->SetInputConnection(geometry->GetOutputPort());
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
      plot->GetProperty()->SetLineWidth(state.contour_line_width);
      Figure::RTAddActor(plot);
      if (state.show_contour_colorbar)
        Figure::RTAddActor2D(BuildColorBar(mapper));
      
      if (state.show_slider)
        AddSlider(interactor,renderer);
      
    } 
  }
  
 
  template <class DATA,class FILTER>
  void ContourBase::RTBuild3D(
    vtkSmartPointer<vtkRenderWindow> window,
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer,
    vtkSmartPointer<DATA> gridfunc)
  {

    double bounds[6];
    gridfunc->GetBounds(bounds);
    
    auto transform =  vtkSmartPointer<vtkTransform>::New();
    double xsize=bounds[1]-bounds[0];
    double ysize=bounds[3]-bounds[2];
    double zsize=bounds[5]-bounds[4];
    
    double xysize=std::max(xsize,ysize);
    double xyzsize=std::max(xysize,zsize);

    // transform everything to [0,1]x[0,1]x[0,1]
    
    if (state.keep_aspect)
    {
      if (xsize>ysize)
        transform->Translate(0,0.5*(xsize-ysize)/xyzsize,0);
      else
        transform->Translate(0.5*(ysize-xsize)/xyzsize,0,0);
      
      transform->Scale(1.0/xyzsize, 1.0/xyzsize,1.0/xyzsize);
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
    
    transform->Translate(-bounds[0],-bounds[2],-bounds[4]);
    
    auto transformFilter =vtkSmartPointer<vtkTransformFilter>::New();
    transformFilter->SetInputDataObject(gridfunc);
    transformFilter->SetTransform(transform);
    transformFilter->Update();

    if (true)
    {
      
      vtkSmartPointer<vtkPlane> plane= vtkSmartPointer<vtkPlane>::New();
      plane->SetOrigin(0.5,0.5,0.5);
      plane->SetNormal(1,0,0);
      
      vtkSmartPointer<vtkCutter> planecut= vtkSmartPointer<vtkCutter>::New();
      planecut->SetInputConnection(transformFilter->GetOutputPort());
      planecut->SetCutFunction(plane);
      
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(planecut->GetOutputPort());
      mapper->UseLookupTableScalarRangeOn();
      mapper->SetLookupTable(surface_lut);
      
      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->SetMapper(mapper);
      Figure::RTAddActor(plot);
      
      // if (show_slice_colorbar)
      //   Figure::RTAddActor2D(BuildColorBar(mapper));
      
    }
    
    
    if (true)
    {
      
      isocontours->SetInputConnection(transformFilter->GetOutputPort());;
      
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(isocontours->GetOutputPort());
      mapper->UseLookupTableScalarRangeOn();
      mapper->SetLookupTable(surface_lut);
      
      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->GetProperty()->SetOpacity(0.5);
      plot->SetMapper(mapper);
      Figure::RTAddActor(plot);
      Figure::RTAddActor2D(BuildColorBar(mapper));
      
    }

    // create outline
    vtkSmartPointer<vtkOutlineFilter>outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
    outlinefilter->SetInputConnection(transformFilter->GetOutputPort());;
    vtkSmartPointer<vtkPolyDataMapper> outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    outlineMapper->SetInputConnection(outlinefilter->GetOutputPort());
    vtkSmartPointer<vtkActor> outline = vtkSmartPointer<vtkActor>::New();
    outline->SetMapper(outlineMapper);
    outline->GetProperty()->SetColor(0, 0, 0);
    Figure::RTAddActor(outline);
    
    
  } 
  





 template void ContourBase::RTBuild2D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>(
    vtkSmartPointer<vtkRenderWindow> window,
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer,
    vtkSmartPointer<vtkRectilinearGrid> data);
  
  template void ContourBase::RTBuild2D<vtkUnstructuredGrid,vtkGeometryFilter>(
    vtkSmartPointer<vtkRenderWindow> window,
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer,
    vtkSmartPointer<vtkUnstructuredGrid> data);


  template void ContourBase::RTBuild3D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>(
    vtkSmartPointer<vtkRenderWindow> window,
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer,
    vtkSmartPointer<vtkRectilinearGrid> data);

  template void ContourBase::RTBuild3D<vtkUnstructuredGrid,vtkGeometryFilter>(
    vtkSmartPointer<vtkRenderWindow> window,
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer,
    vtkSmartPointer<vtkUnstructuredGrid> data);


  template <class GRIDFUNC>
  void ContourBase::ServerRTSend(vtkSmartPointer<Communicator> communicator,
                                 vtkSmartPointer<GRIDFUNC> gridfunc,
                                 vtkSmartPointer<vtkFloatArray> values) 
  {
    communicator->SendCharBuffer((char*)&state,sizeof(state));
    
    if (state.surface_rgbtab_modified)
    {
      SendRGBTable(communicator, surface_rgbtab);
      state.surface_rgbtab_modified=false;
    }

    if (state.contour_rgbtab_modified)
    {
      SendRGBTable(communicator, contour_rgbtab);
      state.contour_rgbtab_modified=false;
    }


    communicator->SendInt(grid_changed);
    if (grid_changed)
      communicator->Send(gridfunc,1,1);
    else
      communicator->Send(values,1,1);
  }

  template <class GRIDFUNC>
  void ContourBase::ClientMTReceive(vtkSmartPointer<Communicator> communicator,
                                 vtkSmartPointer<GRIDFUNC> gridfunc,
                                 vtkSmartPointer<vtkFloatArray> values) 
  {

    communicator->ReceiveCharBuffer((char*)&state,sizeof(state));

    if (state.surface_rgbtab_modified)
    {
      RGBTable new_rgbtab;
      ReceiveRGBTable(communicator, new_rgbtab);
      SetSurfaceRGBTable(new_rgbtab,state.surface_rgbtab_size);
    }

    if (state.contour_rgbtab_modified)
    {
      RGBTable new_rgbtab;
      ReceiveRGBTable(communicator, new_rgbtab);
      SetContourRGBTable(new_rgbtab,state.contour_rgbtab_size);
    }

    communicator->ReceiveInt(grid_changed);
    if (has_data|| grid_changed)
    {
//      gridfunc->Reset();
    }

    if (grid_changed)
      communicator->Receive(gridfunc,1,1);
    else
      communicator->Receive(values,1,1);



    SetVMinMax(state.real_vmin,state.real_vmax);

    gridfunc->Modified();


    has_data=true;
  }
  
  
  template
  void ContourBase::ServerRTSend<vtkRectilinearGrid>(vtkSmartPointer<Communicator> communicator,
                                                     vtkSmartPointer<vtkRectilinearGrid>, 
                                                     vtkSmartPointer<vtkFloatArray>);
  template
  void ContourBase::ClientMTReceive<vtkRectilinearGrid>(vtkSmartPointer<Communicator> communicator,
                                                     vtkSmartPointer<vtkRectilinearGrid>, 
                                                     vtkSmartPointer<vtkFloatArray>);
  
  template
  void ContourBase::ServerRTSend<vtkUnstructuredGrid>(vtkSmartPointer<Communicator> communicator,
                                                         vtkSmartPointer<vtkUnstructuredGrid>,
                                                         vtkSmartPointer<vtkFloatArray>);
  
  template
  void ContourBase::ClientMTReceive<vtkUnstructuredGrid>(vtkSmartPointer<Communicator> communicator,
                                                         vtkSmartPointer<vtkUnstructuredGrid>,
                                                         vtkSmartPointer<vtkFloatArray>);
  

}
