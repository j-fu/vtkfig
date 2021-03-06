/**
    \file vtkfigFigure.h

    Provide user API base class vtkfig::Figure for figures with API to rendering pipelines.
*/

#ifndef VTKFIG_FIGURE_H
#define VTKFIG_FIGURE_H

#include <memory>
#include <vector>

#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkContextActor.h>
#include <vtkActor2D.h>
#include <vtkContourFilter.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkRenderer.h>
#include <vtkCutter.h>
#include <vtkPlane.h>
#include <vtkTransform.h>
#include <vtkCornerAnnotation.h>
#include <vtkGlyphSource2D.h>
#include <vtkArrowSource.h>
#include <vtkIdList.h>
#include <vtkCubeAxesActor2D.h>
#include <vtkTrivialProducer.h>


#include "vtkfigTools.h"
#include "vtkfigDataSet.h"

#include "internals/vtkfigCommunicator.h"

namespace vtkfig
{


  namespace internals
  {
    class vtkfigRenderThread;
    class vtkfigInteractorStyle;
    class vtkfigTimerCallback;  
    class Client;           
  }

  /// Base class for all figures.
  ///
  /// It justs consists of a set of instances of vtkActor which
  /// contains the data  used by vtk. In order to ease coding (?), derived
  /// class data are defined here as well
  /// 
  /// Derived classes just should fill these actors by calling Figure::AddActor
  ///
  /// In this way, any vtk rendering pipeline can be used. 
  /// 
  /// All methods prefixed with "RT" shall be  called only from render thread.
  /// All methods prefixed with "MP" shall be  called from multiprocessing rendering client.
  class Figure
  {

  public:
    
    /// Construct smartpointer to empty figure
    static std::shared_ptr<Figure> New();

    /// Set background color;
    void SetBackground(double r, double g, double b) { bgcolor[0]=r; bgcolor[1]=g; bgcolor[2]=b;}



    template <class V>
    void SetIsolevels(V&v)
    {
      isoline_filter->SetNumberOfContours(v.size());
      for (size_t i=0;i<v.size(); i++)
        isoline_filter->SetValue(i,v[i]);
      isoline_filter->Modified();
      state.isolevels_locked=true;
    }
  
    /// Add Dataset to figure
    ///
    /// \param name Name of scalar or vector to be shown
    void SetData(std::shared_ptr<DataSet> data, const std::string name="");    

  
    /// Add Dataset with mask to figure
    ///
    /// \param name Name of scalar or vector to be shown
    void SetMaskedData(std::shared_ptr<DataSet> data, const std::string name, const std::string maskname);

    ///
    /// Set contour RGB table from RGBtable
    /// 
    /// \param tab RGB table containg data. 
    /// \param lutsize  Size of lookup table created.
    void SetContourRGBTable(RGBTable & tab, int lutsize);

    /// Set figure title
    void SetTitle(std::string xtitle) { title=xtitle;}

    /// Set view volume (e.g. to zoom in to a part)
    void SetViewVolume(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax)   { state.xmin=xmin; state.xmax=xmax; state.ymin=ymin; state.ymax=ymax; state.zmin=zmin; state.zmax=zmax; transform_dirty=true;}
    
    /// Set view volume to dataset bounds (to overview the full dataset)
    void SetViewVolumeToDataSetBounds() { state.xmax=-1; state.xmin=1; transform_dirty=true;}

    /// Set Range of values
    void SetValueRange(double vmin, double vmax){state.vmin_set=vmin; state.vmax_set=vmax;}
    
    /// Accumulate value ranges during run
    void SetAccumulateRange(bool b){ state.accumulate_range=b;}

    ///  Set fixed  xy aspect ratio
    void SetXYAspect(double a) {state.aspect=a;}
    
    /// Keep aspect ratio
    void KeepXYAspect(bool b) {state.keep_aspect=b;}

    ///  InvalidateTransform
    void SetTransformDirty();

    /// Toggle rendering of domain axes
    void ShowDomainAxes(bool b) { state.show_domain_axes=b;}

    /// Toggle rendering of domain bounding box
    void ShowDomainBox(bool b) { state.show_domain_box=b;}

    /// Toggle rendering of domain boundary as transparent surface
    void ShowDomainOutline(bool b) { state.show_domain_outline=b;}

    
    /// Add vtk Actor to renderer showing figure.
    ///
    /// Intended for custom pipeline.
    void AddContextActor(vtkSmartPointer<vtkContextActor> prop) {RTAddContextActor(prop);}

    /// Add vtk Actor to renderer showing figure
    ///
    /// Intended for custom pipeline.
    void AddActor(vtkSmartPointer<vtkActor> prop) {RTAddActor(prop);};

    /// Add vtk Actor to renderer showing figure
    ///
    /// Intended for custom pipeline.
    void AddActor2D(vtkSmartPointer<vtkActor2D> prop) {RTAddActor2D(prop);};

    /// Show string in message field
    ///
    /// Intended for custom pipeline.
    void Message(std::string msg) {RTMessage(msg);}

    /// Add annotations
    ///
    /// Intended for custom pipeline.
    void AddAnnotations(){RTAddAnnotations();}

    
    //!!!protected:
  public:
    friend class Frame;
    friend class internals::vtkfigRenderThread;
    friend class internals::Client;
    friend class internals::vtkfigInteractorStyle;
    friend class internals::vtkfigTimerCallback;

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



    /// Send rgb table to client
    static void MPSendRGBTable(vtkSmartPointer<internals::Communicator> communicator, RGBTable & rgbtab);

    /// Receive rgb table from client.
    static void MPReceiveRGBTable(vtkSmartPointer<internals::Communicator> communicator, RGBTable & rgbtab);
    
    /// Calculate transformation to unit cube
    /// This shall be applied to all data. Camera is fixed.
    void RTCalcTransform();



    /// Set minmax values from data
    void SetVMinMax();

    /// Generate isolevels after minmax data known
    void GenIsolevels();


    /// Subclasses re-implement this in order to build
    /// the vtk rendering pipeline
    virtual void RTBuildVTKPipeline(){};

    /// Build domain pipeline for 2D/3D figures
    /// (box, suface, axes)
    void RTBuildDomainPipeline(vtkSmartPointer<vtkRenderer> renderer);

    /// Duck typing interface allowing to handle different VTK datatypes
    /// with the same code
    template <class DATA>
      void RTBuildDomainPipeline0(vtkSmartPointer<vtkRenderer> renderer);

    /// Default implementation for 2D/3d datasets. 
    virtual void RTBuildAllVTKPipelines(vtkSmartPointer<vtkRenderer> renderer) 
    {
      RTBuildVTKPipeline();
      RTBuildDomainPipeline(renderer);
      RTAddAnnotations();
    };

    /// Pre-Render actions
    virtual void RTPreRender() {};

    /// Send subclass data to client.
    /// Called from rendering thread.
    virtual void ServerMPSend(vtkSmartPointer<internals::Communicator> communicator){} 

    /// Receive subclass data from server
    virtual void ClientMPReceive(vtkSmartPointer<internals::Communicator> communicator) {};
    
    /// Send data to client.
    /// Called from rendering thread.
    void ServerMPSendData(vtkSmartPointer<internals::Communicator> communicator);

    /// Receive data from server
    void ClientMPReceiveData(vtkSmartPointer<internals::Communicator> communicator);


    /// Process keyboard event
    virtual void RTProcessKey(const std::string key);

    /// Process mouse  event
    virtual void RTProcessMove(int dx, int dy);

    /// Trigger necessary actions to set state variables
    void RTRefreshState();
    void RefreshState();

    
    /// Process keyboard and mouse move events for plane section editing
    int RTProcessPlaneKey(const std::string plane,int idim, const std::string key, bool & edit, vtkSmartPointer<vtkCutter> planecut);
    /// Process keyboard and mouse move events for plane section editing
    int RTProcessPlaneMove(const std::string plane,int idim, int dx, int dy, bool & edit, vtkSmartPointer<vtkCutter> planecut );
    /// Process keyboard and mouse move events for plane section editing
    void RTShowPlanePos(vtkSmartPointer<vtkCutter> planecut,const std::string plane,  int idim);

    /// Process keyboard and mouse move events for isolevel editing
    int RTProcessIsoKey(const std::string key, bool & edit);
    /// Process keyboard and mouse move events for isolevel editing
    int RTProcessIsoMove(int dx, int dy, bool & edit);
    /// Process keyboard and mouse move events for isolevel editing
    void RTShowIsolevel();
    /// Process keyboard and mouse move events for isolevel editing
    void RTUpdateIsoSurfaceFilter();

    /// Process keyboard events for quiver arrow editing etc
    int RTProcessArrowKey(const std::string key, bool & edit);

    /// Process mouse move events for quiver arrow editing etc
    int RTProcessArrowMove(int dx, int dy, bool & edit);

    /// Show arrow scale informtion
    void RTShowArrowScale();

    /// Show figure is active  [x] 
    void RTShowActive();

    /// Show figure is inactive  [ ] 
    void RTShowInActive();
    
    /// Check if figure is empty (== no actors added)
    bool IsEmpty();
    
    /// Get subclass name (for s-c communication, should be replaced by tag
    virtual std::string SubClassName() {return std::string("FigureBase");}
    
    /// Update all actors belonging to figure
    void RTUpdateActors();
           
    /// Obtain the data range from the relevant dataset.
    void SetRange();

    /// Title+message text fields
    vtkSmartPointer<vtkCornerAnnotation> annot;

    /// Transformation
    vtkSmartPointer<vtkTransform> transform=0;

    /// Cutter for plane sections
    vtkSmartPointer<vtkCutter> planecutX;
    /// Cutter for plane sections
    vtkSmartPointer<vtkCutter> planecutY;
    /// Cutter for plane sections
    vtkSmartPointer<vtkCutter> planecutZ;

    /// Plane equation for plane sections
    vtkSmartPointer<vtkPlane> planeX;
    /// Plane equation for plane sections
    vtkSmartPointer<vtkPlane> planeY;
    /// Plane equation for plane sections
    vtkSmartPointer<vtkPlane> planeZ;

    /// Arrow glyph source
    vtkSmartPointer<vtkGlyphSource2D> arrow2d;    
    /// Arrow glyph source
    vtkSmartPointer<vtkTransformPolyDataFilter> arrow3d;    
    /// Arrow glyph source
    vtkSmartPointer<vtkTransform> arrow3dt;
    /// Arrow glyph source
    vtkSmartPointer<vtkArrowSource> arrow3ds;

    /// Isosurface plot actor
    vtkSmartPointer<vtkActor>     isosurface_plot;
    /// Isosurface plot filter
    vtkSmartPointer<vtkContourFilter> isosurface_filter;

    /// Isoline plot actor
    vtkSmartPointer<vtkActor>     isoline_plot;
    /// Isoline plot filter
    vtkSmartPointer<vtkContourFilter> isoline_filter;

    /// Items for elevation plot
    vtkSmartPointer<vtkActor>     elevation_plot;

    /// Items for surface plot
    vtkSmartPointer<vtkActor>     surface_plot;

    /// Items for elevation plot
    vtkSmartPointer<vtkTransform> warp_transform;

    /// Axes actor
    vtkSmartPointer<vtkCubeAxesActor2D> axes;

    /// Box actor
    vtkSmartPointer<vtkActor> box;

    /// Outline plot actor
    vtkSmartPointer<vtkActor> outline;




    /// Data producer for grid dataset
    vtkSmartPointer<vtkTrivialProducer> data_producer=NULL;

    /// Data producer for boundary grid dataset
    vtkSmartPointer<vtkTrivialProducer> boundary_data_producer=NULL;

    /// coordinate scale
    double coordinate_scale_factor;

    /// Name of data item in data set 
    std::string dataname;

    /// Cell mask
    vtkSmartPointer<vtkIdList> celllist=0;

    /// Title of figure
    std::string title;

    /// Color lookup table for surface plots
    vtkSmartPointer<vtkLookupTable> surface_lut;

    /// Color table for surface plots
    RGBTable surface_rgbtab{{0,0,0,1},{1,1,0,0}};

    /// Color lookup table for quiver plots
    vtkSmartPointer<vtkLookupTable> quiver_lut;

    /// Color table for quiver plots
    RGBTable quiver_rgbtab{{0,0,0,0},{1,0,0,0}};

    /// Color lookup table for stream plots
    vtkSmartPointer<vtkLookupTable> stream_lut;

    /// Color table for stream plots
    RGBTable stream_rgbtab{{0,0,0,1},{1,1,0,0}};


    /// Color lookup table for elevation plots
    vtkSmartPointer<vtkLookupTable> elevation_lut;

    
    /// Color table for elevation plots
    RGBTable elevation_rgbtab{{0.0,0.9,0.9,0.9},{1.0,0.9,0.9,0.9}};

    
    /// Color lookup table for grid plot cell colors
    vtkSmartPointer<vtkLookupTable> cell_lut;

    
    /// Color table for grid plot cell colors
    RGBTable cell_rgbtab{
      {0.00, 1.0, 0.5, 0.5},
      {0.25, 1.0, 1.0, 0.5},
      {0.50, 0.5, 1.0, 0.5},
      {0.75, 0.5, 1.0, 1.0},
      {1.00, 0.5, 0.5, 1.0}};
    int cell_rgbtab_size=65;


    /// Color lookup table for grid plot boundary colors
    vtkSmartPointer<vtkLookupTable> bface_lut;

    /// Color table for grid plot boundary colors
    RGBTable bface_rgbtab{
      {0.00, 0.8, 0.0, 0.0},
      {0.25, 0.8, 0.8, 0.0},
      {0.50, 0.0, 0.8, 0.0},
      {0.75, 0.0, 0.8, 0.8},
      {1.00, 0.0, 0.0, 0.8}};
    int bface_rgbtab_size=65;

    /// Cell color bar for grid plots.
    vtkSmartPointer<vtkScalarBarActor> cbar=NULL;

    /// Boundary color bar for grid plots.
    vtkSmartPointer<vtkScalarBarActor> bcbar=NULL;

    
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


      /// View volume data 
      double xmin=1.0;
      double xmax=-1.0;
      double ymin=1.0;
      double ymax=-1.0;
      double zmin=1.0;
      double zmax=-1.0;

      double coordinate_scale_factor_xyz[3]={1,1,1};
      
      bool accumulate_range=false;

      bool isolevels_locked=false;

      double eps_geom=1.0e-8;
      
      bool keep_aspect=true;

      int surface_colorbar_num_labels=11;
      
      char surface_colorbar_label_format[32]=" %+9.2e";
      
      double aspect=1.0;

      bool show_grid_edges=true;
      
      bool show_surface=true;
      
      bool show_quiver=true;
      
      bool show_stream=false;
      
      bool show_isolines=true;

      bool show_isosurfaces=false;
      
      bool show_slider=false;
      
      bool show_surface_colorbar=true;

      bool show_grid_colorbar=true;

      bool show_quiver_colorbar=false;

      bool show_stream_colorbar=false;
      
      bool show_contour_colorbar=false;
      
      bool show_elevation=false;

      int contour_rgbtab_size=2;
      
      bool contour_rgbtab_modified=false;

      int surface_rgbtab_size=63;
      
      bool surface_rgbtab_modified=true;

      int quiver_rgbtab_size=2;
      
      bool quiver_rgbtab_modified=true;

      int stream_rgbtab_size=63;
      
      bool stream_rgbtab_modified=true;

      int elevation_rgbtab_size=2;
      
      bool wireframe=false;

      bool show_domain_axes=true;
      
      bool show_domain_outline=true;
      
      bool show_domain_box=true;
      
      double domain_opacity=0.1;
      
      double domain_surface_color[3]={0.8,0.8,0.8};

      int spacedim=2;
      
      double isoline_width=2;
      
      double quiver_arrowscale_user=1.0;

      double quiver_arrowscale_geometry=0.1;

      double stream_maximum_propagation=1;

      int stream_maximum_number_of_steps=500;

      double stream_ribbonwidth=0.01;

      double stream_initial_integration_step=0.001;

      double stream_maximum_integration_step=0.1;

      double warp_ztran=0.0;

      double elevation_scale=1.0;

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


  protected:
    Figure();
    virtual ~Figure(){};
    
    //private:
  public:
    /// The different actor lists
    std::vector<vtkSmartPointer<vtkContextActor>> ctxactors;
    std::vector<vtkSmartPointer<vtkActor>> actors;
    std::vector<vtkSmartPointer<vtkActor2D>> actors2d;

    bool transform_dirty=true;

    /// default background color
    double bgcolor[3]={1,1,1};

    /// position in its respective frame
    int framepos=0;
    
    int framenum=-1;
  };

  

}

#endif
