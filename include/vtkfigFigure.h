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
#include "vtkSliderWidget.h"
#include "vtkSliderRepresentation2D.h"

#include "vtkfigCommunicator.h"
#include "vtkfigTools.h"
#include "vtkContourFilter.h"
#include "vtkPolyDataAlgorithm.h"
#include "vtkRenderer.h"
#include "vtkCutter.h"
#include "vtkPlane.h"
#include "vtkCornerAnnotation.h"

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

    template <class V> 
    void  SetSurfaceRGBTable(const V & tab, int lutsize)
    {
      RGBTable rgbtab;
      rgbtab.resize(tab.size()/4);
      for (int i=0,j=0; i<tab.size(); i+=4,j++)
      {
        rgbtab[j].x=tab[i+0];
        rgbtab[j].r=tab[i+1];
        rgbtab[j].g=tab[i+2];
        rgbtab[j].b=tab[i+3];
      }
      SetSurfaceRGBTable(rgbtab, lutsize);
    }

    void SetSurfaceRGBTable(RGBTable & tab, int lutsize);

    void SetContourRGBTable(RGBTable & tab, int lutsize);

    void SetModelTransform(vtkSmartPointer<vtkRenderer> renderer, int dim, double bounds[6]);

    void ShowSurface(bool b) {state.show_surface=b;}

    void ShowIsocontours(bool b) {state.show_isocontours=b;}

    void ShowSlider(bool b) {state.show_slider=b;}

    void ShowSurfaceColorbar(bool b) {state.show_surface_colorbar=b;}

    void ShowContourColorbar(bool b) {state.show_contour_colorbar=b;}

    void ShowElevation(bool b) {state.show_elevation=b;}

    void SetTitle(std::string xtitle) { title=xtitle;}
    
    void SetValueRange(double vmin, double vmax){state.vmin_set=vmin; state.vmax_set=vmax;}

    void SetNumberOfIsocontours(int n) {state.num_contours=n; state.max_num_contours= std::max(n,state.max_num_contours);}

    void SetIsoContourLineWidth(double w) {state.contour_line_width=w;}

    void SetMaxNumberOfIsoContours(int n) {state.max_num_contours=n;}

    void ShowIsoContoursOnCutplanes(bool b) {state.show_isocontours_on_cutplanes=b;}

    void SetXYAspect(double a) {state.aspect=a;}

    void KeepXYAspect(bool b) {state.keep_aspect=b;}
    
    template< class G> void SetData(G& xgriddata, const std::string xdataname="");
    
    template< class G> void SetData(std::shared_ptr<G> xgriddata, const std::string xdataname="");    

    void RTAddContextActor(vtkSmartPointer<vtkContextActor> prop);

    void RTAddActor(vtkSmartPointer<vtkActor> prop);

    void RTAddActor2D(vtkSmartPointer<vtkActor2D> prop);

    void RTMessage(std::string msg);

    void RTAddAnnotations();


    
  protected:

    friend class Frame;
    friend class MainThread;
    friend class TimerCallback;
    friend class Client;
    friend class MyInteractorStyle;
    friend class MySliderCallback;

    vtkSmartPointer<vtkSliderWidget> sliderWidget;
    vtkSmartPointer<vtkCornerAnnotation> tactor;
    vtkSmartPointer<vtkCutter> planecutX;
    vtkSmartPointer<vtkCutter> planecutY;
    vtkSmartPointer<vtkCutter> planecutZ;

    vtkSmartPointer<vtkPlane> planeX;
    vtkSmartPointer<vtkPlane> planeY;
    vtkSmartPointer<vtkPlane> planeZ;
    
    void AddSlider(vtkSmartPointer<vtkRenderWindowInteractor> i,
                   vtkSmartPointer<vtkRenderer> r);
    
    friend class mySliderCallback;
   

    vtkSmartPointer<vtkDataSet> data=NULL;

    std::string dataname;

    std::string title;

    vtkSmartPointer<vtkContourFilter> isocontours;


    vtkSmartPointer<vtkLookupTable> surface_lut;
    
    vtkSmartPointer<vtkLookupTable> contour_lut;
    
    RGBTable surface_rgbtab{{0,0,0,1},{1,1,0,0}};
    
    RGBTable contour_rgbtab{{0,0,0,0},{1,0,0,0}};
    
    void SetVMinMax(double vmin, double vmax);


    virtual void RTProcessKey(const std::string key);
    virtual void RTProcessMove(int dx, int dy);
    void RTProcessPlaneMove(const std::string plane,int idim, int dx, int dy, bool & edit, vtkSmartPointer<vtkCutter> planecut );
    void RTProcessPlaneKey(const std::string plane,int idim, const std::string key, bool & edit, vtkSmartPointer<vtkCutter> planecut);


    
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

      double panscale=1.0;

      DataType datatype;

    } state;

    struct
    {
      bool x_plane=false;
      bool y_plane=false;
      bool z_plane=false;
      bool l_iso=false;
      bool q_scale=false;
    } edit;   

    double bounds[6];
    double center[3];

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
    title=xdataname;
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
