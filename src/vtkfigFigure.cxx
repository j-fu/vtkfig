#include <vtkTransform.h>
#include <vtkCamera.h>
#include <vtkProperty2D.h>
#include <vtkTextProperty.h>
#include <vtkCommand.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkTransformFilter.h>
#include <vtkOutlineFilter.h>
#include <vtkCubeAxesActor2D.h>
#include <vtkMapper.h>
#include <vtkMapper2D.h>
#include <vtkContextScene.h>


#include "vtkfigFigure.h"
#include "config.h"

namespace vtkfig
{
  ////////////////////////////////////////////////
  Figure::Figure() 
  {


    data_producer=vtkSmartPointer<vtkTrivialProducer>::New();
    boundary_data_producer=vtkSmartPointer<vtkTrivialProducer>::New();

    surface_lut=BuildLookupTable(surface_rgbtab,state.surface_rgbtab_size);
    //contour_lut=BuildLookupTable(contour_rgbtab,state.contour_rgbtab_size);
    elevation_lut=BuildLookupTable(elevation_rgbtab,state.elevation_rgbtab_size);
    quiver_lut=BuildLookupTable(quiver_rgbtab,state.quiver_rgbtab_size);
    stream_lut=BuildLookupTable(stream_rgbtab,state.stream_rgbtab_size);

    cell_lut=BuildLookupTable(cell_rgbtab,cell_rgbtab_size);
    bface_lut=BuildLookupTable(bface_rgbtab,bface_rgbtab_size);



    isoline_filter = vtkSmartPointer<vtkContourFilter>::New();
    isoline_filter->SetNumberOfContours(11);
    isosurface_filter = vtkSmartPointer<vtkContourFilter>::New();


    isosurface_plot = vtkSmartPointer<vtkActor>::New();
    isoline_plot = vtkSmartPointer<vtkActor>::New();
    elevation_plot = vtkSmartPointer<vtkActor>::New();
    surface_plot = vtkSmartPointer<vtkActor>::New();



    planecutX= vtkSmartPointer<vtkCutter>::New();
    planecutY= vtkSmartPointer<vtkCutter>::New();
    planecutZ= vtkSmartPointer<vtkCutter>::New();

    arrow2d = vtkSmartPointer<vtkGlyphSource2D>::New();
    arrow2d->SetGlyphTypeToArrow();

    arrow3ds= vtkSmartPointer<vtkArrowSource>::New();
    arrow3ds->SetTipResolution(16);
    arrow3ds->SetTipLength(0.4);
    arrow3ds->SetTipRadius(0.2);
    arrow3ds->SetShaftRadius(0.1);

    arrow3dt=vtkSmartPointer<vtkTransform>::New();
    arrow3d=vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    arrow3d->SetTransform(arrow3dt);
    arrow3d->SetInputConnection(arrow3ds->GetOutputPort());



    
    planeX= vtkSmartPointer<vtkPlane>::New();
    planeX->SetNormal(1,0,0);
    planeY= vtkSmartPointer<vtkPlane>::New();
    planeY->SetNormal(0,1,0);
    planeZ= vtkSmartPointer<vtkPlane>::New();
    planeZ->SetNormal(0,0,1);


  };


  void Figure::SetRange()
  {
    
    if (SubClassName()=="GridView") return;
    if (SubClassName()=="XYPlot") return;
    if (SubClassName()=="ChartXY") return;
    if (SubClassName()=="FigureBase") return;
    if (SubClassName()=="Surf2D") return;

    auto data=vtkDataSet::SafeDownCast(data_producer->GetOutputDataObject(0));

    auto values=vtkDoubleArray::SafeDownCast(data->GetPointData()->GetAbstractArray(dataname.c_str()));

    if (!values) return;

    double vrange[2];
    // If  comp (2nd argument) is  -1, the  range of  the magnitude  (L2 norm)  over all
    // components  will  be provided.  The  range  is computed  and  then
    // cached,  and  will  not  be re-computed  on  subsequent  calls  to
    // GetRange() unless the array is modified or the requested component
    // changes. THIS METHOD IS NOT THREAD SAFE.
    values->GetRange(vrange,-1);
    state.data_vmin=vrange[0];
    state.data_vmax=vrange[1];
  }


  void Figure::SetData(DataSet& vtkfig_data, const std::string name)
  {

    this->state.spacedim=vtkfig_data.GetSpaceDimension();

    this->data_producer->SetOutput(0);
    this->data_producer->SetOutput(vtkfig_data.GetVTKDataSet());
    this->data_producer->Modified();

    this->boundary_data_producer->SetOutput(0);
    this->boundary_data_producer->SetOutput(vtkfig_data.GetVTKBoundaryDataSet());
    this->boundary_data_producer->Modified();

    this->coordinate_scale_factor=vtkfig_data.coordinate_scale_factor;

    state.datatype=vtkfig_data.GetDataType();

    this->dataname=name;
    this->celllist=0;
    this->title=name;
    this->state.coordinate_scale_factor_xyz[0]=vtkfig_data.coordinate_scale_factor*vtkfig_data.coordinate_scale_factor_xyz[0];
    this->state.coordinate_scale_factor_xyz[1]=vtkfig_data.coordinate_scale_factor*vtkfig_data.coordinate_scale_factor_xyz[1];
    this->state.coordinate_scale_factor_xyz[2]=vtkfig_data.coordinate_scale_factor*vtkfig_data.coordinate_scale_factor_xyz[2];

    // We have to inquire properties of data (ranges etc.) here because
    // in the rendering pipelines they can be used only once
    // In fact, we should find a way to move these things to the 
    // corresponding derived classes. Idea: 
    // virtual method ExploreData() 
    
    // Data for scalar/vector stuff
    SetVMinMax();

    // Data for grid visualization
    auto cr=vtkDoubleArray::SafeDownCast(vtkfig_data.GetVTKDataSet()->GetCellData()->GetAbstractArray("cellregions"));
    if (cr)
    {
      double range[2];
      cr->GetRange(range);
      cell_lut->SetTableRange(range[0],range[1]);
      cell_lut->Modified();
      if (cbar)
      {
        cbar->SetNumberOfLabels((int)(range[1]-range[0]+1));
        cbar->Modified();
      }
    }
    
    // Data for grid visualization
    auto boundary_data=vtkfig_data.GetVTKBoundaryDataSet();
    if  (boundary_data)
    {
      auto bcr=vtkDoubleArray::SafeDownCast(boundary_data->GetCellData()->GetAbstractArray("boundarycellregions"));
      if (bcr)
      {
        double range[2];
        bcr->GetRange(range);
        bface_lut->SetTableRange(range[0],range[1]);
        bface_lut->Modified();
        if (bcbar)
        {
          bcbar->SetNumberOfLabels((int)(range[1]-range[0]+1));
          bcbar->Modified();
        }
      }
    }
      
  }
  
  

  void Figure::SetMaskedData(DataSet& vtkfig_data, const std::string name,const std::string maskname)
  {
    SetData(vtkfig_data,name);
    celllist=vtkfig_data.GetCellList(maskname);
  }

  void Figure::SetData(std::shared_ptr<DataSet> vtkfig_data, const std::string name)
  {
    SetData(*vtkfig_data,name);
  }
  
  

  void Figure::SetMaskedData(std::shared_ptr<DataSet> vtkfig_data, const std::string name,const std::string maskname)
  {
    SetMaskedData(*vtkfig_data,name,maskname);
  }


  void Figure::SendRGBTable(vtkSmartPointer<internals::Communicator> communicator, RGBTable & rgbtab)
  {
    communicator->SendInt(rgbtab.size());
    communicator->SendDoubleBuffer((double*)rgbtab.data(),rgbtab.size()*sizeof(RGBPoint)/sizeof(double));
  }
  
  void Figure::ReceiveRGBTable(vtkSmartPointer<internals::Communicator> communicator, RGBTable & rgbtab)
  {
    int tabsize;
    communicator->ReceiveInt(tabsize);
    rgbtab.resize(tabsize);
    communicator->ReceiveDoubleBuffer((double*)rgbtab.data(),rgbtab.size()*sizeof(RGBPoint)/sizeof(double));
  }






  void Figure::RTAddActor(vtkSmartPointer<vtkActor> prop) {actors.push_back(prop);}
  void Figure::RTAddActor2D(vtkSmartPointer<vtkActor2D> prop) {actors2d.push_back(prop);}
  void Figure::RTAddContextActor(vtkSmartPointer<vtkContextActor> prop) {ctxactors.push_back(prop);}
  bool Figure::IsEmpty(void) {return (actors.size()==0 && actors2d.size()==0 && ctxactors.size()==0);}
  



  void Figure::RTShowPlanePos(vtkSmartPointer<vtkCutter> planecut, const std::string plane, int idim)
  {
    //if (this->SubClassName()!="ScalarView") return;
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
    //if (this->SubClassName()!="ScalarView") return;

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
  // if (this->SubClassName()!="VectorView") return;

    RTMessage("arrow_scale="+std::to_string(state.quiver_arrowscale_user));
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
      state.isolevels_locked=true;
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

    if (key=="I" && state.spacedim==3)
    {
//      state.show_isolines=!state.show_isolines;
      state.show_isosurfaces=!state.show_isosurfaces;
      //      isoline_plot->SetVisibility(state.show_isolines);
      isosurface_plot->SetVisibility(state.show_isosurfaces);
      RTUpdateIsoSurfaceFilter();
      isosurface_plot->Modified();
      return;
    }

    if (key=="S" && state.spacedim==2)
    {
      state.show_surface=!state.show_surface;
      state.show_isolines=!state.show_isolines;
      isoline_plot->SetVisibility(state.show_isolines);
      surface_plot->SetVisibility(state.show_surface);
      isoline_plot->Modified();
      surface_plot->Modified();
      return;
    }

    if (key=="E" && state.spacedim==2)
    {
      state.show_elevation=!state.show_elevation;
      elevation_plot->SetVisibility(state.show_elevation);
      elevation_plot->Modified();
      return;
    }
    
    if (key=="A")
    {
      if (axes) 
      {
        int vis=axes->GetVisibility();
        vis=!vis;
        axes->SetVisibility(vis);
        if (outline)
          outline->SetVisibility(vis);
      }
      return;
    }
    
    if (key=="O")
    {
      if (splot)
      {
        int vis=splot->GetVisibility();
        vis=!vis;
        splot->SetVisibility(vis);
      }
      return;
    }
    



    if (key=="L")
    {
      isoline_filter->SetNumberOfContours(11);
      state.isolevels_locked=false;
      GenIsolevels();
      if (edit.l_iso)
      {
        state.isolevels_locked=true;
        RTShowIsolevel();
      }
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
      double ascale=state.quiver_arrowscale_user;
      ascale*=pow(10.0,((double)dy)/100.0);
      ascale=std::min(ascale,1.0e20);
      ascale=std::max(ascale,1.0e-20);
      state.quiver_arrowscale_user=ascale;
      RTShowArrowScale();
      arrow3dt->Identity();
      double scalefac=state.quiver_arrowscale_geometry*state.quiver_arrowscale_user/state.real_vmax;
      arrow3dt->Scale(scalefac,scalefac,scalefac);
      arrow2d->SetScale(scalefac);
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

  
  
  
  void Figure::SetContourRGBTable(RGBTable & tab, int tabsize)
  {
    state.contour_rgbtab_size=tabsize;
    state.contour_rgbtab_modified=true;
    //contour_rgbtab=tab;
    //contour_lut=BuildLookupTable(tab,tabsize);
  }
  
  void Figure::RTCalcTransform()
  {
    if (!transform_dirty) return;
    auto data=vtkDataSet::SafeDownCast(data_producer->GetOutputDataObject(0));

    if (  state.xmin<state.xmax 
          &&state.ymin<state.ymax
          &&(state.spacedim==2 || state.zmin<state.zmax))
    {
      data_bounds[0]=state.xmin;
      data_bounds[1]=state.xmax;
      data_bounds[2]=state.ymin;
      data_bounds[3]=state.ymax;
      data_bounds[4]=state.zmin;
      data_bounds[5]=state.zmax;
      data_center[0]=0.5*(data_bounds[0]+data_bounds[1]);
      data_center[1]=0.5*(data_bounds[2]+data_bounds[3]);
      data_center[2]=0.5*(data_bounds[4]+data_bounds[5]);
    }
    else if (data)
    {
      data->GetBounds(data_bounds);
      data->GetCenter(data_center);
    }
    else
    {
      throw std::runtime_error("Specify either view volume or dataset for figure\n");
    }
    if (!transform)
      transform =  vtkSmartPointer<vtkTransform>::New();
    transform->Identity();
    double xsize=data_bounds[1]-data_bounds[0];
    double ysize=data_bounds[3]-data_bounds[2];
    double zsize=data_bounds[5]-data_bounds[4];
    if (state.spacedim==2) zsize=0;
    double xysize=std::max(xsize,ysize);
    double xyzsize=std::max(xysize,zsize);

    // we don't need this if we prevent vectors from being
    // transformed 
    //state.quiver_arrowscale_geometry=1.0/xyzsize;
    
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
    
    transform_dirty=false;
  }
  
  void Figure::SetVMinMax()
  {
    if (
      SubClassName()!="ScalarView"
      && 
      SubClassName()!="VectorView"
      ) return;

    this->SetRange();
      
    if (state.vmin_set<state.vmax_set)
    {
      state.real_vmin=state.vmin_set;
      state.real_vmax=state.vmax_set;
    }
    else if (state.accumulate_range)
    {
      state.real_vmin=std::min(state.real_vmin,state.data_vmin);
      state.real_vmax=std::max(state.real_vmax,state.data_vmax);
    }
    else
    {
      state.real_vmin=state.data_vmin;
      state.real_vmax=state.data_vmax;
    }
    // cout << SubClassName() << endl;
    // cout << state.data_vmin << " " << state.data_vmax << endl;
    // cout << state.real_vmin << " " << state.real_vmax << endl << endl;

    arrow3dt->Identity();
    double scalefac=state.quiver_arrowscale_geometry*state.quiver_arrowscale_user/(state.real_vmax);
    arrow3dt->Scale(scalefac,scalefac,scalefac);
    arrow3dt->Modified();
    arrow2d->SetScale(scalefac);
    arrow2d->Modified();

    double lut_min=state.real_vmin;
    double lut_max=state.real_vmax;

    
    if (SubClassName()=="ScalarView")
    {
      surface_lut->SetTableRange(lut_min,lut_max);
      surface_lut->Modified();
      GenIsolevels();
    }

    if (SubClassName()=="VectorView")
    {
      quiver_lut->SetTableRange(lut_min,lut_max);
      quiver_lut->Modified();

      stream_lut->SetTableRange(lut_min,lut_max);
      stream_lut->Modified();
    }



    // contour_lut->SetTableRange(state.real_vmin,state.real_vmax);
    // contour_lut->Modified();
  }

  void Figure::GenIsolevels()
  {
    if (!state.isolevels_locked)
    {
      double eps = this->state.eps_geom*(state.real_vmax-state.real_vmin)/(isoline_filter->GetNumberOfContours());
      isoline_filter->GenerateValues(isoline_filter->GetNumberOfContours(), state.real_vmin+eps, state.real_vmax-eps);
      isoline_filter->Modified();
    }
    RTUpdateIsoSurfaceFilter();
  }

  // this will see the right text size
  void Figure::RTInitAnnotations()
  {
    if (!annot)
    {

      annot= vtkSmartPointer<vtkCornerAnnotation>::New();
      auto textprop=annot->GetTextProperty();
      annot->SetMinimumFontSize(8);
      annot->SetMaximumFontSize(20);
      textprop->ItalicOff();
      textprop->BoldOn();
      textprop->SetFontSize(8);
      textprop->SetFontFamilyToCourier();
      textprop->SetColor(0,0,0);
    }

  }
  void Figure::RTAddAnnotations()
  {
    RTInitAnnotations();
    annot->SetText(7,title.c_str());
    annot->SetText(4,"");
    annot->SetText(3,"[ ]");
    Figure::RTAddActor2D(annot);
  }


  void Figure::RTShowActive()
  {
    RTInitAnnotations();
    annot->SetText(3,"[X]");
    annot->Modified();
  }
  
  
  void Figure::RTShowInActive()
  {
    RTInitAnnotations();
    annot->SetText(3,"[ ]");
    annot->Modified();
  }

  void Figure::RTMessage(std::string msg)
  {
    RTInitAnnotations();
    annot->SetText(4,msg.c_str());
    annot->Modified();
  }
      
  
  void Figure::RTUpdateActors()
  {
    SetVMinMax();
    if (SubClassName()!="XYPlot")
      RTCalcTransform();

    for (auto actor: actors) {auto m=actor->GetMapper(); if (m) m->Update();}
    for (auto actor: ctxactors) {auto m=actor->GetScene(); if (m) m->SetDirty(true);}
    for (auto actor: actors2d){auto m=actor->GetMapper(); if (m) m->Update();}
  }


  void Figure::ServerRTSendData(vtkSmartPointer<internals::Communicator> communicator) 
  {
    auto data=vtkDataSet::SafeDownCast(data_producer->GetOutputDataObject(0));

    if (SubClassName()!="XYPlot")
    {
      communicator->SendCharBuffer((char*)&state,sizeof(state));
      communicator->SendString(dataname);
      communicator->Send(data,1,1);
    }
    ServerRTSend(communicator);
  };

  void Figure::ClientMTReceiveData(vtkSmartPointer<internals::Communicator> communicator)
  {
    //auto data=vtkDataSet::SafeDownCast(data_producer->GetOutputDataObject(0));
    
    if (SubClassName()!="XYPlot")
    {

      communicator->ReceiveCharBuffer((char*)&state,sizeof(state));
      communicator->ReceiveString(dataname);

      vtkSmartPointer<vtkDataSet> data;
      if (state.datatype==DataSet::DataType::RectilinearGrid)
        data=vtkSmartPointer<vtkRectilinearGrid>::New();
      else if (state.datatype==DataSet::DataType::UnstructuredGrid)
        data=vtkSmartPointer<vtkUnstructuredGrid>::New();
      communicator->Receive(data,1,1);
      data_producer->SetOutput(data);
    }
    ClientMTReceive(communicator);
  }
  



  /// \todo add filter to extract boundary edges
  template <class DATA>
  void Figure::RTBuildDomainPipeline0(vtkSmartPointer<vtkRenderer> renderer)
  {
    RTCalcTransform();

    auto geometry=vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
    geometry->SetInputConnection(data_producer->GetOutputPort());
    
    auto transgeometry=vtkSmartPointer<vtkTransformFilter>::New();
    transgeometry->SetInputConnection(geometry->GetOutputPort());
    transgeometry->SetTransform(transform);


    /// if boundary cell color set, use this one!
    if (state.show_domain_boundary && state.spacedim==3)
    {
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(transgeometry->GetOutputPort());
      splot=vtkSmartPointer<vtkActor>::New();
      if (state.spacedim==3)
      {
        splot->GetProperty()->SetOpacity(state.domain_opacity);
        splot->GetProperty()->SetColor(state.domain_surface_color);
      }
      else // TODO add filter to extract boundary edges
      {
        splot->GetProperty()->SetOpacity(1.0);
        splot->GetProperty()->SetColor(0,0,0);
      }

      splot->SetMapper(mapper);
      Figure::RTAddActor(splot);
    }

    
    if (state.show_domain_box&& state.spacedim==3)
    {
      // create outline
      vtkSmartPointer<vtkOutlineFilter>outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
      outlinefilter->SetInputConnection(transgeometry->GetOutputPort());
      vtkSmartPointer<vtkPolyDataMapper> outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      outlineMapper->SetInputConnection(outlinefilter->GetOutputPort());
      outline = vtkSmartPointer<vtkActor>::New();
      outline->SetMapper(outlineMapper);
      outline->GetProperty()->SetColor(0, 0, 0);
      Figure::RTAddActor(outline);
    }


    if (state.show_domain_axes)
    {
      axes=vtkSmartPointer<vtkCubeAxesActor2D>::New();
      double unscaled_data_bounds[6];
      unscaled_data_bounds[0]=data_bounds[0]/this->state.coordinate_scale_factor_xyz[0];
      unscaled_data_bounds[1]=data_bounds[1]/this->state.coordinate_scale_factor_xyz[0];
      unscaled_data_bounds[2]=data_bounds[2]/this->state.coordinate_scale_factor_xyz[1];
      unscaled_data_bounds[3]=data_bounds[3]/this->state.coordinate_scale_factor_xyz[1];
      unscaled_data_bounds[4]=data_bounds[4]/this->state.coordinate_scale_factor_xyz[2];
      unscaled_data_bounds[5]=data_bounds[5]/this->state.coordinate_scale_factor_xyz[2];
      axes->SetRanges(unscaled_data_bounds);
      axes->SetUseRanges(1);
      axes->SetInputConnection(transgeometry->GetOutputPort());
      axes->GetProperty()->SetColor(0, 0, 0);
      axes->SetFontFactor(1.5);
      axes->SetCornerOffset(0); 
      axes->SetNumberOfLabels(3); 
      axes->SetInertia(100);
      axes->SetLabelFormat("%6.2g");

      ///!!!
      axes->SetCamera(renderer->GetActiveCamera());
      
      if (state.spacedim==2)
      {
        axes->SetXLabel("");
        axes->SetYLabel("");
        axes->ZAxisVisibilityOff();
      }
      else
      {
        axes->SetXLabel("x");
        axes->SetYLabel("y");
        axes->SetZLabel("z");
      }
      auto textprop=axes->GetAxisLabelTextProperty();
      textprop->ItalicOff();
      textprop->BoldOn();
      textprop->SetFontFamilyToCourier();
      textprop->SetColor(0,0,0);

      textprop=axes->GetAxisTitleTextProperty();
      textprop->ItalicOff();
      textprop->BoldOn();
      textprop->SetFontFamilyToCourier();
      textprop->SetColor(0,0,0);
      
      Figure::RTAddActor2D(axes);
    }
  } 
  

  /////////////////////////////////////////////////////////////////////
  /// Generic access to filter
  void  Figure::RTBuildDomainPipeline(vtkSmartPointer<vtkRenderer> renderer)
  {
    if (state.datatype==DataSet::DataType::UnstructuredGrid)
      this->RTBuildDomainPipeline0<vtkUnstructuredGrid>(renderer); 
    else if (state.datatype==DataSet::DataType::RectilinearGrid)
      this->RTBuildDomainPipeline0<vtkRectilinearGrid>(renderer);
  }
  


}
