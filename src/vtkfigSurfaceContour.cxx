#include "vtkSliderRepresentation2D.h"
#include "vtkProperty2D.h"
#include "vtkAlgorithmOutput.h"
#include "vtkTextProperty.h"
#include "vtkRectilinearGrid.h"
#include "vtkGeometryFilter.h"
#include "vtkRectilinearGridGeometryFilter.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridGeometryFilter.h"
#include "vtkPlane.h"
#include "vtkCutter.h"
#include "vtkImplicitBoolean.h"
#include "vtkOutlineFilter.h"
#include "vtkCubeAxesActor2D.h"
#include "vtkCamera.h"
#include "vtkAppendPolyData.h"


#include "vtkfigSurfaceContour.h"


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
    SurfaceContour *contour2d;
  };



  void SurfaceContour::AddSlider(vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                                vtkSmartPointer<vtkRenderer> renderer)
  {
      if (true)
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
  void SurfaceContour::RTBuild2D(
    vtkSmartPointer<vtkRenderWindow> window,
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer,
    vtkSmartPointer<DATA> gridfunc)
  {
    double bounds[6];
    gridfunc->GetBounds(bounds);
    renderer->GetActiveCamera()->SetParallelProjection(1);

    auto geometry=vtkSmartPointer<FILTER>::New();
    geometry->SetInputDataObject(gridfunc);
    
    SetModelTransform(renderer,bounds);

        
    if (state.show_surface)
    {
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(geometry->GetOutputPort());
      
      mapper->UseLookupTableScalarRangeOn();
      mapper->SetLookupTable(surface_lut);
      mapper->ImmediateModeRenderingOn();
      
      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->SetMapper(mapper);
      Figure::RTAddActor(plot);
      
      if (state.show_surface_colorbar)
        Figure::RTAddActor2D(BuildColorBar(mapper));
    }
    
    
    if (state.show_isocontours)
    {
      
      isocontours->SetInputConnection(geometry->GetOutputPort());
      
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
      
      AddSlider(interactor,renderer);
      
    } 

    if (true)
    {
      auto axes=vtkSmartPointer<vtkCubeAxesActor2D>::New();
      axes->SetInputConnection(geometry->GetOutputPort());
      axes->GetProperty()->SetColor(0, 0, 0);
      axes->SetFontFactor(1.25);
      axes->SetCornerOffset(0); 
      axes->SetNumberOfLabels(3); 

      axes->SetCamera(renderer->GetActiveCamera());
      axes->ZAxisVisibilityOff();
      axes->SetXLabel("");
      axes->SetYLabel("");

      auto textprop=axes->GetAxisLabelTextProperty();
      textprop->ItalicOff();
      textprop->SetFontFamilyToArial();
      textprop->SetColor(0,0,0);

      textprop=axes->GetAxisTitleTextProperty();
      textprop->ItalicOff();
      textprop->SetFontFamilyToArial();
      textprop->SetColor(0,0,0);

      Figure::RTAddActor2D(axes);
    }
    

  }
  
 
  template <class DATA,class FILTER>
  void SurfaceContour::RTBuild3D(
    vtkSmartPointer<vtkRenderWindow> window,
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer,
    vtkSmartPointer<DATA> gridfunc)
  {

    double bounds[6];
    gridfunc->GetBounds(bounds);
    double center[3];
    gridfunc->GetCenter(center);
    Figure::SetModelTransform(renderer,bounds);


    auto planeX= vtkSmartPointer<vtkPlane>::New();
    planeX->SetOrigin(bounds[0],0,0);
    planeX->SetNormal(1,0,0);
      
    auto planeY= vtkSmartPointer<vtkPlane>::New();
    planeY->SetOrigin(center);
    planeY->SetNormal(0,1,0);
      
    auto planeZ= vtkSmartPointer<vtkPlane>::New();
    planeZ->SetOrigin(center);
    planeZ->SetNormal(0,0,1);
    
    
    auto planecutX= vtkSmartPointer<vtkCutter>::New();
    planecutX->SetInputDataObject(gridfunc);
    planecutX->SetCutFunction(planeX);

      
    auto planecutY= vtkSmartPointer<vtkCutter>::New();
    planecutY->SetInputDataObject(gridfunc);
    planecutY->SetCutFunction(planeY);
      
    auto planecutZ= vtkSmartPointer<vtkCutter>::New();
    planecutZ->SetInputDataObject(gridfunc);
    planecutZ->SetCutFunction(planeZ);
      

    auto xyz =    vtkSmartPointer<vtkAppendPolyData>::New();
    xyz->SetUserManagedInputs(1);
    xyz->SetNumberOfInputs(3);

    xyz->SetInputConnectionByNumber(0,planecutX->GetOutputPort());
    xyz->SetInputConnectionByNumber(1,planecutY->GetOutputPort());
    xyz->SetInputConnectionByNumber(2,planecutZ->GetOutputPort());



      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(xyz->GetOutputPort());
      mapper->UseLookupTableScalarRangeOn();
      mapper->SetLookupTable(surface_lut);
      
      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->GetProperty()->SetOpacity(1);
      plot->SetMapper(mapper);
      Figure::RTAddActor(plot);
      
      //if (show_slice_colorbar)
      //   Figure::RTAddActor2D(BuildColorBar(mapper));
      
      Figure::RTAddActor2D(BuildColorBar(mapper));

    
    
    if (state.show_isocontours)
    {
      

      if (state.show_isocontours_on_cutplanes)
        isocontours->SetInputConnection(xyz->GetOutputPort());
      else
        isocontours->SetInputDataObject(gridfunc);
      
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(isocontours->GetOutputPort());
      mapper->UseLookupTableScalarRangeOn();
      mapper->SetLookupTable(contour_lut);
      
      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      if (state.show_isocontours_on_cutplanes)
        plot->GetProperty()->SetOpacity(1.0);
      else
        plot->GetProperty()->SetOpacity(0.4);
      plot->GetProperty()->SetLineWidth(state.contour_line_width);
      plot->SetMapper(mapper);
      Figure::RTAddActor(plot);
      
      AddSlider(interactor,renderer);
    }



    
    if (true)
    {
      // create outline
      vtkSmartPointer<vtkOutlineFilter>outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
      outlinefilter->SetInputDataObject(gridfunc);;
      vtkSmartPointer<vtkPolyDataMapper> outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      outlineMapper->SetInputConnection(outlinefilter->GetOutputPort());
      vtkSmartPointer<vtkActor> outline = vtkSmartPointer<vtkActor>::New();
      outline->SetMapper(outlineMapper);
      outline->GetProperty()->SetColor(0, 0, 0);
      Figure::RTAddActor(outline);
    }

    if (true)
    {
      auto axes=vtkSmartPointer<vtkCubeAxesActor2D>::New();
      axes->SetInputData(gridfunc);
      axes->GetProperty()->SetColor(0, 0, 0);
      axes->SetFontFactor(1.0);
      axes->SetCornerOffset(0); 
      axes->SetNumberOfLabels(3); 
      axes->SetInertia(100);

      axes->SetCamera(renderer->GetActiveCamera());
      axes->SetXLabel("x");
      axes->SetYLabel("y");
      axes->SetZLabel("z");

      auto textprop=axes->GetAxisLabelTextProperty();
      textprop->ItalicOff();
      textprop->SetFontFamilyToArial();
      textprop->SetColor(0,0,0);

      textprop=axes->GetAxisTitleTextProperty();
      textprop->ItalicOff();
      textprop->SetFontFamilyToArial();
      textprop->SetColor(0,0,0);

      Figure::RTAddActor2D(axes);
    }
  } 
  

  void  SurfaceContour::RTBuild(
    vtkSmartPointer<vtkRenderWindow> window,
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer)
  {
    data->GetPointData()->SetActiveAttribute(dataname.c_str(),vtkDataSetAttributes::SCALARS);
    
    if (state.datatype==Figure::DataType::UnstructuredGrid)
      {
        auto griddata=vtkUnstructuredGrid::SafeDownCast(data);
        
        if (state.spacedim==2)
          this->RTBuild2D<vtkUnstructuredGrid,vtkGeometryFilter>(window, interactor,renderer,griddata);
        else
          this->RTBuild3D<vtkUnstructuredGrid,vtkGeometryFilter>(window,interactor,renderer,griddata); 
      }
      else if (state.datatype==Figure::DataType::RectilinearGrid)
      {
        auto griddata=vtkRectilinearGrid::SafeDownCast(data);
        
     
        if (state.spacedim==2)
          this->RTBuild2D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>(window, interactor,renderer,griddata);
        else
          this->RTBuild3D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>(window, interactor,renderer,griddata);
      }
    }




  void SurfaceContour::ServerRTSend(vtkSmartPointer<Communicator> communicator)
  {
    communicator->SendCharBuffer((char*)&state,sizeof(state));
    communicator->SendString(dataname);

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

    communicator->Send(data,1,1);
  }

  void SurfaceContour::ClientMTReceive(vtkSmartPointer<Communicator> communicator)
  {

    communicator->ReceiveCharBuffer((char*)&state,sizeof(state));
    communicator->ReceiveString(dataname);

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

    if (data==NULL)
    {
      if (state.datatype==Figure::DataType::RectilinearGrid)
        data=vtkSmartPointer<vtkRectilinearGrid>::New();
      else if (state.datatype==Figure::DataType::UnstructuredGrid)
        data=vtkSmartPointer<vtkUnstructuredGrid>::New();
    }
    communicator->Receive(data,1,1);

    SetVMinMax(state.real_vmin,state.real_vmax);

    data->Modified();
    data->GetPointData()->SetActiveAttribute(dataname.c_str(),vtkDataSetAttributes::SCALARS);
    data->GetPointData()->GetScalars()->Modified();

  }
  
  
}
