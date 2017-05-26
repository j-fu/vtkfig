#include "vtkTransform.h"
#include "vtkCamera.h"

#include "vtkfigFigure.h"
namespace vtkfig
{
  ////////////////////////////////////////////////
  Figure::Figure() 
  {
    surface_lut=BuildLookupTable(surface_rgbtab,state.surface_rgbtab_size);
    contour_lut=BuildLookupTable(contour_rgbtab,state.contour_rgbtab_size);
    isocontours = vtkSmartPointer<vtkContourFilter>::New();
  };

  void Figure::RTAddActor(vtkSmartPointer<vtkActor> prop) {actors.push_back(prop);}
  void Figure::RTAddActor2D(vtkSmartPointer<vtkActor2D> prop) {actors2d.push_back(prop);}
  void Figure::RTAddContextActor(vtkSmartPointer<vtkContextActor> prop) {ctxactors.push_back(prop);}
  bool Figure::IsEmpty(void) {return (actors.size()==0 && actors2d.size()==0&& ctxactors.size()==0);}
  
  
  
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
  
  void Figure::SetModelTransform(vtkSmartPointer<vtkRenderer> renderer, int dim, double bounds[6])
  {
    auto transform =  vtkSmartPointer<vtkTransform>::New();
    double xsize=bounds[1]-bounds[0];
    double ysize=bounds[3]-bounds[2];
    double zsize=bounds[5]-bounds[4];
    
    double xysize=std::max(xsize,ysize);
    double xyzsize=std::max(xysize,zsize);
    
    if (dim==2)
      transform->Scale(1.2,1.2,0);
      
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
    
    renderer->GetActiveCamera()->SetModelTransformMatrix(transform->GetMatrix());
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
    
    double tempdiff = (state.real_vmax-state.real_vmin)/(1.0e8*state.num_contours);
    isocontours->GenerateValues(state.num_contours, state.real_vmin+tempdiff, state.real_vmax-tempdiff);
    isocontours->Modified();
    
  }
  
  void Figure::RTUpdateActors()
  {
    for (auto actor: actors) {auto m=actor->GetMapper(); if (m) m->Update();}
    for (auto actor: ctxactors) {auto m=actor->GetScene(); if (m) m->SetDirty(true);}
    for (auto actor: actors2d){auto m=actor->GetMapper(); if (m) m->Update();}
  }


}
