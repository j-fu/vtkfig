#ifndef VTKFIG_FIGURE_H
#define VTKFIG_FIGURE_H

#include <memory>
#include <vector>

#include "vtkSmartPointer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkActor.h"
#include "vtkContextActor.h"
#include "vtkContextScene.h"
#include "vtkActor2D.h"
#include "vtkMapper.h"
#include "vtkMapper2D.h"

#include "vtkfigCommunicator.h"
#include "vtkfigTools.h"
#include "vtkContourFilter.h"
#include "vtkPolyDataAlgorithm.h"
#include "vtkTransform.h"
#include "vtkCamera.h"
#include "vtkRenderer.h"


namespace vtkfig
{

  /// Base class for all figures.
  ///
  /// It justs consists of a set of instances of vtkActor which
  /// contains the data  used by vtk.
  /// 
  /// Derived classes just should fill these actors by calling Figure::AddActor
  ///
  /// In this way, any vtk rendering pipeline can be used. 
  /// 

  template <typename  T> class Parameter
  {
    T val=0;
    bool mod=true;
    public:
  Parameter(T val): val(val){};
  Parameter(): Parameter(0){};
    void set(T v) { val =v; mod=true;}
    T get(void) { mod=false; return val;}
  };

  template<typename T>
    inline std::ostream & operator << (std::ostream & s, Parameter<T> &p)
  { s << p.get(); return s;}


  class Figure
  {
    friend class Frame;
    friend class MainThread;
    friend class TimerCallback;
    friend class Client;
    friend class InteractorStyleTrackballCamera;

  public:
    Figure();
    static std::shared_ptr<Figure> New() { return std::make_shared<Figure>(); }
    ~Figure()
    {
    }

    void SetBackground(double r, double g, double b) { bgcolor[0]=r; bgcolor[1]=g; bgcolor[2]=b;}


    void SetSurfaceRGBTable(RGBTable & tab, int tabsize)
    {
      state.surface_rgbtab_size=tabsize;
      state.surface_rgbtab_modified=true;
      surface_rgbtab=tab;
      surface_lut=BuildLookupTable(tab,tabsize);
    }

    void SetContourRGBTable(RGBTable & tab, int tabsize)
    {
      state.contour_rgbtab_size=tabsize;
      state.contour_rgbtab_modified=true;
      contour_rgbtab=tab;
      contour_lut=BuildLookupTable(tab,tabsize);
    }

    void SetModelTransform(vtkSmartPointer<vtkRenderer> renderer, double bounds[6])
    {
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
      
      renderer->GetActiveCamera()->SetModelTransformMatrix(transform->GetMatrix());
    }




    void ShowSurface(bool b) {state.show_surface=b;}
    void ShowIsocontours(bool b) {state.show_isocontours=b;}
    void ShowSlider(bool b) {state.show_slider=b;}
    void ShowSurfaceColorbar(bool b) {state.show_surface_colorbar=b;}
    void ShowContourColorbar(bool b) {state.show_contour_colorbar=b;}
    void ShowElevation(bool b) {state.show_elevation=b;}
    void SetValueRange(double vmin, double vmax){state.vmin_set=vmin; state.vmax_set=vmax; SetVMinMax(vmin,vmax);}
    void SetNumberOfIsocontours(int n) {state.num_contours=n; state.max_num_contours= std::max(n,state.max_num_contours);}
    void SetIsoContourLineWidth(double w) {state.contour_line_width=w;}

    void SetMaxNumberOfIsoContours(int n) {state.max_num_contours=n;}
    void ShowIsoContoursOnCutplanes(bool b) {state.show_isocontours_on_cutplanes=b;}
    void SetXYAspect(double a) {state.aspect=a;}
    void KeepXYAspect(bool b) {state.keep_aspect=b;}

    void RTAddContextActor(vtkSmartPointer<vtkContextActor> prop);

    void RTAddActor(vtkSmartPointer<vtkActor> prop);

    void RTAddActor2D(vtkSmartPointer<vtkActor2D> prop);

    

    
  protected:

    vtkSmartPointer<vtkContourFilter> isocontours;


    vtkSmartPointer<vtkLookupTable> surface_lut;
    vtkSmartPointer<vtkLookupTable> contour_lut;
    RGBTable surface_rgbtab{{0,0,0,1},{1,1,0,0}};
    RGBTable contour_rgbtab{{0,0,0,0},{1,0,0,0}};


    void SetVMinMax(double vmin, double vmax)
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

    
    /// All functions here are to be called from render thread.
    virtual void RTBuild(
      vtkSmartPointer<vtkRenderWindow> window,
      vtkSmartPointer<vtkRenderWindowInteractor> interactor,
      vtkSmartPointer<vtkRenderer> renderer) {};


    bool IsEmpty();

    virtual std::string SubClassName() {return std::string("unknown");}
    
    virtual void ServerRTSend(vtkSmartPointer<Communicator> communicator) {};

    virtual void ClientMTReceive(vtkSmartPointer<Communicator> communicator) {};

    void RTUpdateActors()
    {
      for (auto actor: actors) {auto m=actor->GetMapper(); if (m) m->Update();}
      for (auto actor: ctxactors) {auto m=actor->GetScene(); if (m) m->SetDirty(true);}
      for (auto actor: actors2d){auto m=actor->GetMapper(); if (m) m->Update();}
    }

    enum class DataType
    {
      None=0,
        RectilinearGrid=1,
        UnstructuredGrid=2
    };


    struct
    {
      double vmin_set=1.0e100;
      double vmax_set=-1.0e100;
      double real_vmin=0;
      double real_vmax=1;
      
      int num_contours=10;
      int max_num_contours=10;
      
      bool keep_aspect=true;
      double aspect=1.0;
      
      bool show_surface=true;
      bool show_isocontours=true;
      bool show_slider=false;
      bool show_surface_colorbar=true;
      bool show_contour_colorbar=false;
      bool show_elevation=true;

      int contour_rgbtab_size=2;
      bool contour_rgbtab_modified=true;

      int surface_rgbtab_size=255;
      bool surface_rgbtab_modified=true;
      bool wireframe=false;
      int spacedim=2;
      double contour_line_width=2;
      bool show_isocontours_on_cutplanes=true;
      DataType datatype;
    } state;

    
  private:
    std::vector<vtkSmartPointer<vtkContextActor>> ctxactors;
    std::vector<vtkSmartPointer<vtkActor>> actors;
    std::vector<vtkSmartPointer<vtkActor2D>> actors2d;
    double bgcolor[3]={1,1,1};
    int framepos=0;

    

  };
  
};

#endif
