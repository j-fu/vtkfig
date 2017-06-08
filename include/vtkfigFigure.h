#ifndef VTKFIG_FIGURE_H
#define VTKFIG_FIGURE_H

#include <memory>
#include <vector>

#include "vtkSmartPointer.h"
#include "vtkActor.h"
#include "vtkContextActor.h"
#include "vtkContextScene.h"
#include "vtkActor2D.h"
#include "vtkMapper.h"
#include "vtkMapper2D.h"
#include "vtkContourFilter.h"
#include "vtkPolyDataAlgorithm.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkRenderer.h"
#include "vtkCutter.h"
#include "vtkPlane.h"
#include "vtkTransform.h"
#include "vtkCornerAnnotation.h"
#include "vtkGlyphSource2D.h"
#include "vtkArrowSource.h"
#include "vtkIdList.h"

#include "vtkfigCommunicator.h"
#include "vtkfigTools.h"
#include "vtkfigDataSet.h"

namespace vtkfig
{


  namespace internals
  {
    class MainThread;
    class MyInteractorStyle;
    class MyTimerCallback;  
    class MySliderCallback;  
    class Client;           
  }

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
    
    /// Construct empty figure
    Figure();
    
    /// Construct smartpointer to empty figure
    static std::shared_ptr<Figure> New() { return std::make_shared<Figure>(); }

    ~Figure(){}

    /// Set background color;
    void SetBackground(double r, double g, double b) { bgcolor[0]=r; bgcolor[1]=g; bgcolor[2]=b;}



  
    /// Add Dataset to figure
    ///
    /// \param name Name of scalar or vector to be shown
    void SetData(std::shared_ptr<DataSet> data, const std::string name="");    

  
    /// Add Dataset with mask to figure
    ///
    /// \param name Name of scalar or vector to be shown
    void SetMaskedData(std::shared_ptr<DataSet> data, const std::string name, const std::string maskname);

    /// Add Dataset to figure
    ///
    /// \param name Name of scalar or vector to be shown
    void SetData(DataSet *data, const std::string name="");    

  
    /// Add Dataset with mask to figure
    ///
    /// \param name Name of scalar or vector to be shown
    void SetMaskedData(DataSet *data, const std::string name, const std::string maskname);


    ///
    /// Set contour RGB table from RGBtable
    /// 
    /// \param tab RGB table containg data. 
    /// \param lutsize  Size of lookup table created.
    void SetContourRGBTable(RGBTable & tab, int lutsize);

    /// Set figure title
    void SetTitle(std::string xtitle) { title=xtitle;}


    /// Set Range of values
    void SetValueRange(double vmin, double vmax){state.vmin_set=vmin; state.vmax_set=vmax;}
    
    /// Accumulate value ranges during run
    void SetAccumulateRange(bool b){ state.accumulate_range=b;}

    ///  Set fixed  xy aspect ratio
    void SetXYAspect(double a) {state.aspect=a;}
    
    /// Keep aspect ratio
    void KeepXYAspect(bool b) {state.keep_aspect=b;}


    /// Toggle rendering of domain axes
    void ShowDomainAxes(bool b) { state.show_domain_axes=b;}

    /// Toggle rendering of domain bounding box
    void ShowDomainBox(bool b) { state.show_domain_box=b;}

    /// Toggle rendering of domain boundary as transparent surface
    void ShowDomainBoundary(bool b) { state.show_domain_boundary=b;}

    
    /// Add vtk Actor to renderer showing figure
    ///
    /// Usually, this is called from the render thread, it
    /// can however be useful in custom pipelines
    void RTAddContextActor(vtkSmartPointer<vtkContextActor> prop);

    /// Add vtk Actor to renderer showing figure
    ///
    /// Usually, this is called from the render thread, it
    /// can however be useful in custom pipelines
    void RTAddActor(vtkSmartPointer<vtkActor> prop);

    /// Add vtk Actor to renderer showing figure
    ///
    /// Usually, this is called from the render thread, it
    /// can however be useful in custom pipelines
    void RTAddActor2D(vtkSmartPointer<vtkActor2D> prop);

    /// Show string in message field
    ///
    /// Usually, this is called from the render thread, it
    /// can however be useful in custom pipelines
    void RTMessage(std::string msg);

    /// Add annotations
    ///
    /// Usually, this is called from the render thread, it
    /// can however be useful in custom pipelines
    void RTAddAnnotations();

    /// Init annotations
    /// 
    /// Called by RTAddAnnotations() or RTMessage()
    void RTInitAnnotations();


    
  protected:

    friend class Frame;
    friend class internals::MainThread;
    friend class internals::Client;
    friend class internals::MyInteractorStyle;
    friend class internals::MyTimerCallback;
    friend class internals::MySliderCallback;

    /// Send rgb table to client
    static void SendRGBTable(vtkSmartPointer<internals::Communicator> communicator, RGBTable & rgbtab);

    /// Receive rgb table from client.
    static void ReceiveRGBTable(vtkSmartPointer<internals::Communicator> communicator, RGBTable & rgbtab);


    /// The following items are declared in the base
    /// class in order to allow easy coding of interaction
    
    /// Title+message text fields
    vtkSmartPointer<vtkCornerAnnotation> annot;

    /// Cutters for plane sections
    vtkSmartPointer<vtkCutter> planecutX;
    vtkSmartPointer<vtkCutter> planecutY;
    vtkSmartPointer<vtkCutter> planecutZ;

    /// Plane equations for plane sections
    vtkSmartPointer<vtkPlane> planeX;
    vtkSmartPointer<vtkPlane> planeY;
    vtkSmartPointer<vtkPlane> planeZ;

    /// Arrow glyph source
    vtkSmartPointer<vtkGlyphSource2D> arrow2d;    
    vtkSmartPointer<vtkTransformPolyDataFilter> arrow3d;    
    vtkSmartPointer<vtkTransform> arrow3dt;

    /// Items for isosurface plot
    vtkSmartPointer<vtkActor>     isosurface_plot;
    vtkSmartPointer<vtkContourFilter> isosurface_filter;

    /// Items for isoline plot
    vtkSmartPointer<vtkActor>     isoline_plot;
    vtkSmartPointer<vtkContourFilter> isoline_filter;

    /// Items for elevation plot
    vtkSmartPointer<vtkActor>     elevation_plot;

    /// Items for surface plot
    vtkSmartPointer<vtkActor>     surface_plot;




    /// Calculate transformation to unit cube
    /// This shall be applied to all data. Camera is fixed.
    vtkSmartPointer<vtkTransform> CalcTransform(vtkSmartPointer<vtkDataSet> data);


    /// Data set visualized
    vtkSmartPointer<vtkDataSet> data=NULL;

    vtkfig::DataSet* vtkfig_dataset=NULL;

    /// Name of data item in data set 
    std::string dataname;

    /// Cell mask
    vtkSmartPointer<vtkIdList> celllist=0;

    /// Title of figure
    std::string title;

    /// Color lookup table for surface plots
    vtkSmartPointer<vtkLookupTable> surface_lut;
    RGBTable surface_rgbtab{{0,0,0,1},{1,1,0,0}};
    
    /// Color lookup table for contour plots
    /// vtkSmartPointer<vtkLookupTable> contour_lut;
    /// RGBTable contour_rgbtab{{0,0,0,0},{1,0,0,0}};
    
    /// Color lookup table for elevation plots
    vtkSmartPointer<vtkLookupTable> elevation_lut;
    RGBTable elevation_rgbtab{{0.0,0.9,0.9,0.9},{1.0,0.9,0.9,0.9}};
    
    /// Set minmax values from data
    void SetVMinMax();

    /// Generate isolevels after minmax data known
    void GenIsolevels();

    /// All functions prefixed with "RT" shall be  called only from render thread.


    /// Subclasses re-implement this in order to build
    /// the vtk rendering pipeline
    virtual void RTBuildVTKPipeline(){};



    void RTBuildDomainPipeline(vtkSmartPointer<vtkRenderer> renderer);

    template <class DATA>
      void RTBuildDomainPipeline(vtkSmartPointer<vtkRenderer> renderer,vtkSmartPointer<DATA> gridfunc);

    void RTBuildAllVTKPipelines(vtkSmartPointer<vtkRenderer> renderer) 
    {
      RTBuildVTKPipeline();
      RTBuildDomainPipeline(renderer);
      RTAddAnnotations();
    };



    /// These two need to re-implemented in subclasses 
    /// in order to get server-client communication
    virtual void ServerRTSend(vtkSmartPointer<internals::Communicator> communicator) {};
    virtual void ClientMTReceive(vtkSmartPointer<internals::Communicator> communicator) {};


    /// Process keyboard and mouse move events
    void RTProcessKey(const std::string key);
    void RTProcessMove(int dx, int dy);
    
    /// Process keyboard and mouse move events for plane section editing
    int RTProcessPlaneKey(const std::string plane,int idim, const std::string key, bool & edit, vtkSmartPointer<vtkCutter> planecut);
    int RTProcessPlaneMove(const std::string plane,int idim, int dx, int dy, bool & edit, vtkSmartPointer<vtkCutter> planecut );
    void RTShowPlanePos(vtkSmartPointer<vtkCutter> planecut,const std::string plane,  int idim);

    /// Process keyboard and mouse move events for isolevel editing
    int RTProcessIsoKey(const std::string key, bool & edit);
    int RTProcessIsoMove(int dx, int dy, bool & edit);
    void RTShowIsolevel();
    void RTUpdateIsoSurfaceFilter();

    /// Process keyboard and mouse move events for quiver arrow editing
    int RTProcessArrowKey(const std::string key, bool & edit);
    int RTProcessArrowMove(int dx, int dy, bool & edit);
    void RTShowArrowScale();

    
    /// Check if figure is empty (== no actors added)
    bool IsEmpty();
    
    /// Get subclass name (for s-c communication, should be replaced by tag
    virtual std::string SubClassName() {return std::string("unknown");}
    
    
    /// Update all actors belonging to figure
    void RTUpdateActors();
           
    
    
    /// figure state
    struct
    {

      /// min/max set through api
      double vmin_set=1.0e100;
      double vmax_set=-1.0e100;
      
      /// min/max generated from api and data
      double real_vmin=0;
      double real_vmax=1;
      
      /// min/max calculated from data
      double data_vmin=0;
      double data_vmax=1;
      
      bool accumulate_range=false;

      bool isolevels_locked=false;

      /// number of isocontours
      int num_contours=11;
      int max_num_contours=11;

      double eps_geom=1.0e-8;
      
      bool keep_aspect=true;
      
      double aspect=1.0;
      
      bool show_surface=true;
      
      bool show_isolines=true;

      bool show_isosurfaces=false;
      
      bool show_slider=false;
      
      bool show_surface_colorbar=true;
      
      bool show_contour_colorbar=false;
      
      bool show_elevation=false;

      int contour_rgbtab_size=2;
      
      bool contour_rgbtab_modified=false;

      int surface_rgbtab_size=255;
      
      bool surface_rgbtab_modified=true;

      int elevation_rgbtab_size=2;

      
      bool wireframe=false;

      bool show_domain_axes=true;
      
      bool show_domain_boundary=true;
      
      bool show_domain_box=true;
      
      double domain_opacity=0.1;
      
      double domain_surface_color[3]={0.8,0.8,0.8};

      int spacedim=2;
      
      double isoline_width=2;
      
      double quiver_arrow_scale=0.333;

      double quiver_surface_distance=1.0e-10;

      double streamcolor[3]={0.8,0.8,0.8};

      double streamlength=1.0;

      double streamribbonwidth=0.01;

      DataSet::DataType datatype;

    } state;


    /// edit state
    struct
    {
      bool x_plane=false;
      bool y_plane=false;
      bool z_plane=false;
      bool l_iso=false;
      bool a_scale=false;
    } edit;   

    double data_bounds[6];
    double data_center[3];

    double trans_bounds[6];
    double trans_center[3];

  private:

    /// The different actor lists
    std::vector<vtkSmartPointer<vtkContextActor>> ctxactors;
    std::vector<vtkSmartPointer<vtkActor>> actors;
    std::vector<vtkSmartPointer<vtkActor2D>> actors2d;

    /// default background color
    double bgcolor[3]={1,1,1};

    /// position in its respective frame
    int framepos=0;
  };

  

}

#endif
