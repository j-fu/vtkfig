#include "vtkTransform.h"
#include "vtkCamera.h"
#include "vtkProperty2D.h"
#include "vtkTextProperty.h"
#include "vtkCommand.h"

#include "vtkfigFigure.h"
namespace vtkfig
{
  ////////////////////////////////////////////////
  Figure::Figure() 
  {



    surface_lut=BuildLookupTable(surface_rgbtab,state.surface_rgbtab_size);
    contour_lut=BuildLookupTable(contour_rgbtab,state.contour_rgbtab_size);
    elevation_lut=BuildLookupTable(elevation_rgbtab,state.elevation_rgbtab_size);
    isoline_filter = vtkSmartPointer<vtkContourFilter>::New();
    isosurface_filter = vtkSmartPointer<vtkContourFilter>::New();
    sliderWidget = vtkSmartPointer<vtkSliderWidget>::New();


    isosurface_plot = vtkSmartPointer<vtkActor>::New();
    isoline_plot = vtkSmartPointer<vtkActor>::New();
    elevation_plot = vtkSmartPointer<vtkActor>::New();
    surface_plot = vtkSmartPointer<vtkActor>::New();


    planecutX= vtkSmartPointer<vtkCutter>::New();
    planecutY= vtkSmartPointer<vtkCutter>::New();
    planecutZ= vtkSmartPointer<vtkCutter>::New();


    // make a vector glyph
    arrow = vtkSmartPointer<vtkGlyphSource2D>::New();
    arrow->SetGlyphTypeToArrow();

    
    planeX= vtkSmartPointer<vtkPlane>::New();
    planeX->SetNormal(1,0,0);
    planeY= vtkSmartPointer<vtkPlane>::New();
    planeY->SetNormal(0,1,0);
    planeZ= vtkSmartPointer<vtkPlane>::New();
    planeZ->SetNormal(0,0,1);


    
    



    annot= vtkSmartPointer<vtkCornerAnnotation>::New();
    auto textprop=annot->GetTextProperty();
    textprop->ItalicOff();
    textprop->BoldOff();
    textprop->SetFontSize(8);
    textprop->SetFontFamilyToArial();
    textprop->SetColor(0,0,0);
  };

  void Figure::SetData(DataSet& xgriddata, const std::string xdataname)
  {
    state.spacedim=xgriddata.GetSpaceDimension();
    data=xgriddata.GetVTKDataSet();
    state.datatype=xgriddata.GetDataType();
    dataname=xdataname;
    title=xdataname;
  }
  
  
  void Figure::SetData(std::shared_ptr<DataSet> xgriddata, const std::string xdataname)
  {
    SetData(*xgriddata,xdataname);
  }


  void Figure::SendRGBTable(vtkSmartPointer<Communicator> communicator, RGBTable & rgbtab)
  {
    communicator->SendInt(rgbtab.size());
    communicator->SendFloatBuffer((float*)rgbtab.data(),rgbtab.size()*sizeof(RGBPoint)/sizeof(float));
  }
  
  void Figure::ReceiveRGBTable(vtkSmartPointer<Communicator> communicator, RGBTable & rgbtab)
  {
    int tabsize;
    communicator->ReceiveInt(tabsize);
    rgbtab.resize(tabsize);
    communicator->ReceiveFloatBuffer((float*)rgbtab.data(),rgbtab.size()*sizeof(RGBPoint)/sizeof(float));
  }

  /////////////////////////////////////////////////////////////////////
  /// Slider callback class

  class MySliderCallback : public vtkCommand
  {
  public:
    static MySliderCallback *New() 
    {
      return new MySliderCallback;
    }
    virtual void Execute(vtkObject *caller, unsigned long, void*)
    {
      vtkSliderWidget *sliderWidget =         reinterpret_cast<vtkSliderWidget*>(caller);
      double value=static_cast<vtkSliderRepresentation *>(sliderWidget->GetRepresentation())->GetValue();
      
      figure->state.num_contours=value;
      figure->SetVMinMax(figure->state.real_vmin,figure->state.real_vmax);
    }
    MySliderCallback():figure(0) {}

    Figure *figure;

  };



  /////////////////////////////////////////////////////////////////////
  /// Slider handling
  void Figure::AddSlider(vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                                vtkSmartPointer<vtkRenderer> renderer)
  {

    auto sliderRep = vtkSmartPointer<vtkSliderRepresentation2D>::New();
    
    cout << "Add slider" << endl;
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
    
    sliderWidget->CreateDefaultRepresentation();    
    sliderWidget->SetRepresentation(sliderRep);

    auto callback =   vtkSmartPointer<MySliderCallback>::New();
    callback->figure = this;
    sliderWidget->AddObserver(vtkCommand::InteractionEvent,callback);
    sliderWidget->SetDefaultRenderer(renderer);
//   sliderWidget->SetCurrentRenderer(renderer);
    sliderWidget->SetInteractor(interactor);
    sliderWidget->SetAnimationModeToAnimate();
    sliderWidget->EnabledOn();
  }



  void Figure::RTAddActor(vtkSmartPointer<vtkActor> prop) {actors.push_back(prop);}
  void Figure::RTAddActor2D(vtkSmartPointer<vtkActor2D> prop) {actors2d.push_back(prop);}
  void Figure::RTAddContextActor(vtkSmartPointer<vtkContextActor> prop) {ctxactors.push_back(prop);}
  bool Figure::IsEmpty(void) {return (actors.size()==0 && actors2d.size()==0&& ctxactors.size()==0);}
  



  void Figure::RTShowPlanePos(vtkSmartPointer<vtkCutter> planecut, const std::string plane, int idim)
  {
    int i=planecut->GetNumberOfContours();
    if (i>0)
    {
      double planepos=data_bounds[2*idim]+ (data_bounds[2*idim+1]-data_bounds[2*idim])*(planecut->GetValue(i-1)+trans_center[idim]-trans_bounds[2*idim]);
      RTMessage("plane_" + plane +"[" + std::to_string(i-1) + "]=" + std::to_string(planepos));
    }
    else
    {
      double planepos=data_bounds[2*idim]+ (data_bounds[2*idim+1]-data_bounds[2*idim])*(trans_center[idim]-trans_bounds[2*idim]);
      RTMessage("[Return] for plane_"+plane+"[0]="+std::to_string(planepos));
    }
  }


  void Figure::RTShowIsolevel()
  {
    int i=isoline_filter->GetNumberOfContours();
    if (i>0)
    {
      double isolevel=isoline_filter->GetValue(i-1);
      RTMessage("isolevel["+ std::to_string(i-1)+ "]="+std::to_string(isolevel));
    }
    else
    {
      RTMessage("[Return] for isolevel[0]="+std::to_string(0.5*(state.real_vmin+state.real_vmax)));
    }
  }

  void Figure::RTShowArrowScale()
  {
    RTMessage("arrow_scale="+std::to_string(state.qv_arrow_scale));
  }

  
  int Figure::RTProcessPlaneKey(
    const std::string plane,
    int idim,
    const std::string key,  
    bool & edit, 
    vtkSmartPointer<vtkCutter> planecut)
  {
    
    if (!edit && key==plane)
    {
      edit=true;     
      RTShowPlanePos(planecut,plane,idim);
      return 1;
    }

    if (edit && key=="Return")
    {
      double planepos;
      int i=planecut->GetNumberOfContours();
      if (i>0)
        planepos=planecut->GetValue(i-1);
      else
        planepos=0.0;
      planecut->SetValue(i,planepos);
      RTShowPlanePos(planecut,plane,idim);
      return 1;
    }

    if (edit&& key=="BackSpace")
    {
      int i=planecut->GetNumberOfContours();
      if (i>0)
        planecut->SetNumberOfContours(i-1);

      RTShowPlanePos(planecut,plane,idim);
      return 1;
    }
    
    if (edit&& key=="Escape")
    {
      edit=false;
      return 1;
    }
    
    return 0;
  }

  int Figure::RTProcessIsoKey(const std::string key,bool & edit)
  {
    if (!edit && key=="l")
    {
      edit=true;        
      RTShowIsolevel();
      return 1;
    }
    
    if (edit && key=="Return")
    {
      double isolevel;
      int i=isoline_filter->GetNumberOfContours();
      if (i>0)
        isolevel=isoline_filter->GetValue(i);
      else
        isolevel=0.5*(state.real_vmin+state.real_vmax);

      isoline_filter->SetValue(i,isolevel);
      RTShowIsolevel();
      RTUpdateIsoSurfaceFilter();
      return 1;
    }
    
    if (edit&& key=="BackSpace")
    {
      int i=isoline_filter->GetNumberOfContours();
      if (i>0)
        isoline_filter->SetNumberOfContours(i-1);
      RTShowIsolevel();
      RTUpdateIsoSurfaceFilter();
      return 1;
    }
    
    if (edit&& key=="Escape")
    {
      edit=false;
      return 1;
    }
    
    return 0;
  }


  int Figure::RTProcessArrowKey(const std::string key,bool & edit)
  {
    if (!edit && key=="a")
    {
      edit=true;        
      RTShowArrowScale();
      return 1;
    }
    
    if (edit&& key=="Escape")
    {
      edit=false;
      return 1;
    }
    
    return 0;
  }

  
  void Figure::RTProcessKey(const std::string key)
  {

    if (key=="i" && state.spacedim==3)
    {
//      state.show_isolines=!state.show_isolines;
      state.show_isosurfaces=!state.show_isosurfaces;
      //      isoline_plot->SetVisibility(state.show_isolines);
      isosurface_plot->SetVisibility(state.show_isosurfaces);
      RTUpdateIsoSurfaceFilter();
      isosurface_plot->Modified();
      return;
    }

    if (key=="s" && state.spacedim==2)
    {
      state.show_surface=!state.show_surface;
      state.show_isolines=!state.show_isolines;
      isoline_plot->SetVisibility(state.show_isolines);
      surface_plot->SetVisibility(state.show_surface);
      isoline_plot->Modified();
      surface_plot->Modified();
      return;
    }

    if (key=="e" && state.spacedim==2)
    {
      state.show_elevation=!state.show_elevation;
      elevation_plot->SetVisibility(state.show_elevation);
      elevation_plot->Modified();
      return;
    }


    if (key=="L")
    {
      state.num_contours=11;
      GenIsolevels();
      if (edit.l_iso)
        RTShowIsolevel();
      return;
    }

    if (
      (edit.x_plane||edit.y_plane|| edit.z_plane|| edit.l_iso  || edit.a_scale)
      &&(key=="x"|| key=="y"|| key=="z"|| key=="l" || key=="a")
      )
    {
      edit.x_plane=false;
      edit.y_plane=false;
      edit.z_plane=false;
      edit.l_iso=false;
      edit.a_scale=false;
    }

    if (RTProcessArrowKey(key,edit.a_scale)) return;
    if (RTProcessIsoKey(key,edit.l_iso)) return;
    if (RTProcessPlaneKey("x",0,key,edit.x_plane, planecutX)) return;
    if (RTProcessPlaneKey("y",1,key,edit.y_plane, planecutY)) return;
    if (RTProcessPlaneKey("z",2,key,edit.z_plane, planecutZ)) return;
  }
  

  int Figure::RTProcessPlaneMove(const std::string plane,int idim, int dx, int dy, bool & edit, 
                                  vtkSmartPointer<vtkCutter> planecut )
  {
    if (edit)
    {
      int i=planecut->GetNumberOfContours()-1;
      if (i>=0)
      {
        double planepos=planecut->GetValue(i)+trans_center[idim];
        planepos+=(0.01)*((double)dx)*(trans_bounds[2*idim+1]-trans_bounds[2*idim+0]);
        planepos=std::min(planepos,trans_bounds[2*idim+1]);
        planepos=std::max(planepos,trans_bounds[2*idim+0]);
        planecut->SetValue(i,planepos-trans_center[idim+0]);
        RTShowPlanePos(planecut,plane,idim);
        planecut->Modified();
      }
      return 1;
    }
    return 0;
  }

  int Figure::RTProcessIsoMove(int dx, int dy, bool & edit)
  {
    if (edit)
    {
      int i=isoline_filter->GetNumberOfContours()-1;
      if (i>=0)
      {
        double isolevel=isoline_filter->GetValue(i);
        isolevel+=(0.01)*((double)dx)*(state.real_vmax-state.real_vmin);
        isolevel=std::min(isolevel,state.real_vmax);
        isolevel=std::max(isolevel,state.real_vmin);
        isoline_filter->SetValue(i,isolevel);
        isoline_filter->Modified();
        RTShowIsolevel();
        RTUpdateIsoSurfaceFilter();
      }
      return 1;
    }
    return 0;
  }
  
  int Figure::RTProcessArrowMove(int dx, int dy, bool & edit)
  {
    if (edit)
    {
      double ascale=state.qv_arrow_scale;
      ascale*=pow(10.0,((double)dx)/10.0);
      ascale=std::min(ascale,1.0e20);
      ascale=std::max(ascale,1.0e-20);
      state.qv_arrow_scale=ascale;
      RTShowArrowScale();
      arrow->SetScale(state.qv_arrow_scale);
      return 1;
    }
    return 0;
  }
  
  void Figure::RTUpdateIsoSurfaceFilter()
  {
    if (state.spacedim==3) 
    {
      int n=isoline_filter->GetNumberOfContours();
      isosurface_filter->SetNumberOfContours(n);
      for (int i=0;i<n;i++)
        isosurface_filter->SetValue(i,isoline_filter->GetValue(i));
    }
  }

  void Figure::RTProcessMove(int dx, int dy)
  {
    if (RTProcessArrowMove(dx,dy,edit.a_scale)) return;
    if (RTProcessIsoMove(dx,dy,edit.l_iso)) return;
    if (RTProcessPlaneMove("x",0,dx,dy,edit.x_plane, planecutX)) return;
    if (RTProcessPlaneMove("y",1,dx,dy,edit.y_plane, planecutY)) return;
    if (RTProcessPlaneMove("z",2,dx,dy,edit.z_plane, planecutZ)) return;
  }

  
  
  void Figure::SetSurfaceRGBTable(RGBTable & tab, int tabsize)
  {
    state.surface_rgbtab_size=tabsize;
    state.surface_rgbtab_modified=true;
    surface_rgbtab=tab;
    surface_lut=BuildLookupTable(tab,tabsize);
  }
  
  void Figure::SetContourRGBTable(RGBTable & tab, int tabsize)
  {
    state.contour_rgbtab_size=tabsize;
    state.contour_rgbtab_modified=true;
    contour_rgbtab=tab;
    contour_lut=BuildLookupTable(tab,tabsize);
  }
  
  vtkSmartPointer<vtkTransform>  Figure::CalcTransform(vtkSmartPointer<vtkDataSet> data)
  {
    data->GetBounds(data_bounds);
    data->GetCenter(data_center);

    auto transform =  vtkSmartPointer<vtkTransform>::New();
    auto glyphtransform =  vtkSmartPointer<vtkTransform>::New();
    double xsize=data_bounds[1]-data_bounds[0];
    double ysize=data_bounds[3]-data_bounds[2];
    double zsize=data_bounds[5]-data_bounds[4];
    if (state.spacedim==2) zsize=0;
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
    
    transform->Translate(-data_bounds[0],-data_bounds[2],-data_bounds[4]);
  
    double p0[3]={data_bounds[0],data_bounds[2],data_bounds[4]};
    double p1[3]={data_bounds[1],data_bounds[3],data_bounds[5]};
    double tp0[3],tp1[3];

    transform->TransformPoint(p0,tp0);
    transform->TransformPoint(p1,tp1);
    trans_bounds[0]=tp0[0];
    trans_bounds[1]=tp1[0];
    trans_bounds[2]=tp0[1];
    trans_bounds[3]=tp1[1];
    trans_bounds[4]=tp0[2];
    trans_bounds[5]=tp1[2];

    transform->TransformPoint(data_center,trans_center);

    return transform;
  }
  
  void Figure::SetVMinMax(double vmin, double vmax)
  {
    if (state.vmin_set<state.vmax_set)
    {
      state.real_vmin=state.vmin_set;
      state.real_vmax=state.vmax_set;
    }
    else
    {
      state.real_vmin=vmin;
      state.real_vmax=vmax;
    }
    
    surface_lut->SetTableRange(state.real_vmin,state.real_vmax);
    surface_lut->Modified();
    contour_lut->SetTableRange(state.real_vmin,state.real_vmax);
    contour_lut->Modified();
  }

  void Figure::GenIsolevels()
  {
    double tempdiff = (state.real_vmax-state.real_vmin)/(1.0e8*state.num_contours);
    isoline_filter->GenerateValues(state.num_contours, state.real_vmin+tempdiff, state.real_vmax-tempdiff);
    isoline_filter->Modified();
    RTUpdateIsoSurfaceFilter();
  }


  void Figure::RTAddAnnotations()
  {
    annot->SetText(7,title.c_str());
    annot->SetText(4,"");
    Figure::RTAddActor2D(annot);
  }

  void Figure::RTMessage(std::string msg)
  {
    annot->SetText(4,msg.c_str());
    annot->Modified();
  }
      
  
  void Figure::RTUpdateActors()
  {
    for (auto actor: actors) {auto m=actor->GetMapper(); if (m) m->Update();}
    for (auto actor: ctxactors) {auto m=actor->GetScene(); if (m) m->SetDirty(true);}
    for (auto actor: actors2d){auto m=actor->GetMapper(); if (m) m->Update();}
  }


}
