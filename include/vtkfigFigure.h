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


  class Figure
  {

  public:
    Figure();

    static std::shared_ptr<Figure> New() { return std::make_shared<Figure>(); }

    ~Figure(){}

    void SetBackground(double r, double g, double b) { bgcolor[0]=r; bgcolor[1]=g; bgcolor[2]=b;}


    void SetSurfaceRGBTable(RGBTable & tab, int tabsize);

    void SetContourRGBTable(RGBTable & tab, int tabsize);

    void SetModelTransform(vtkSmartPointer<vtkRenderer> renderer, double bounds[6]);

    void ShowSurface(bool b) {state.show_surface=b;}

    void ShowIsocontours(bool b) {state.show_isocontours=b;}

    void ShowSlider(bool b) {state.show_slider=b;}

    void ShowSurfaceColorbar(bool b) {state.show_surface_colorbar=b;}

    void ShowContourColorbar(bool b) {state.show_contour_colorbar=b;}

    void ShowElevation(bool b) {state.show_elevation=b;}

    void SetValueRange(double vmin, double vmax){state.vmin_set=vmin; state.vmax_set=vmax; SetVMinMax(vmin,vmax);}

    void SetNumberOfIsocontours(int n) {state.num_contours=n; state.max_num_contours= std::max(n,state.max_num_contours);  SetVMinMax(state.real_vmin,state.real_vmax);}

    void SetIsoContourLineWidth(double w) {state.contour_line_width=w;}

    void SetMaxNumberOfIsoContours(int n) {state.max_num_contours=n;}

    void ShowIsoContoursOnCutplanes(bool b) {state.show_isocontours_on_cutplanes=b;}

    void SetXYAspect(double a) {state.aspect=a;}

    void KeepXYAspect(bool b) {state.keep_aspect=b;}
    
    template< class G> void SetData(G& xgriddata, const std::string xdataname);
    
    template< class G> void SetData(std::shared_ptr<G> xgriddata, const std::string xdataname);    

    void RTAddContextActor(vtkSmartPointer<vtkContextActor> prop);

    void RTAddActor(vtkSmartPointer<vtkActor> prop);

    void RTAddActor2D(vtkSmartPointer<vtkActor2D> prop);

    
  protected:

    friend class Frame;
    friend class MainThread;
    friend class TimerCallback;
    friend class Client;
    friend class InteractorStyleTrackballCamera;


    vtkSmartPointer<vtkDataSet> data=NULL;

    std::string dataname;

    vtkSmartPointer<vtkContourFilter> isocontours;


    vtkSmartPointer<vtkLookupTable> surface_lut;
    
    vtkSmartPointer<vtkLookupTable> contour_lut;
    
    RGBTable surface_rgbtab{{0,0,0,1},{1,1,0,0}};
    
    RGBTable contour_rgbtab{{0,0,0,0},{1,0,0,0}};
    
    void SetVMinMax(double vmin, double vmax);
    
    /// All functions here are to be called from render thread.
    virtual void RTBuild(
      vtkSmartPointer<vtkRenderWindow> window,
      vtkSmartPointer<vtkRenderWindowInteractor> interactor,
      vtkSmartPointer<vtkRenderer> renderer) {};
    
    
    bool IsEmpty();
    
    virtual std::string SubClassName() {return std::string("unknown");}
    
    virtual void ServerRTSend(vtkSmartPointer<Communicator> communicator) {};
    
    virtual void ClientMTReceive(vtkSmartPointer<Communicator> communicator) {};
    
    void RTUpdateActors();
    
    
    enum class DataType
    {
      Notype=0,
        
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

  template< class G> inline void Figure::SetData(G& xgriddata, const std::string xdataname)
  {
    state.spacedim=xgriddata.spacedim;
    data=xgriddata.griddata;
    dataname=xdataname;
    if (data->IsA("vtkUnstructuredGrid"))
      state.datatype=Figure::DataType::UnstructuredGrid;
    else  if (data->IsA("vtkRectilinearGrid"))
      state.datatype=Figure::DataType::RectilinearGrid;
  }
  
  
  template< class G> inline void Figure::SetData(std::shared_ptr<G> xgriddata, const std::string xdataname)
  {
    SetData(*xgriddata,xdataname);
  }

  
};



#endif
