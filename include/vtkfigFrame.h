///
/// \file vtkfigFrame.h
///
/// Define vtkfig::Frame class  providing a window for rendering.
/// 
#ifndef VTKFIG_FRAME_H
#define VTKFIG_FRAME_H

#include <memory>
#include <vector>
#include <set>


#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkCornerAnnotation.h>
#include <vtkOggTheoraWriter.h>

#include "vtkfigCommunicator.h"

namespace vtkfig
{
  class Figure;

  namespace internals
  {
    class MainThread;
    class MyInteractorStyle;
    class MyTimerCallback;  
    class Client;           
  }

  ///
  /// Provide a framework wrapping window+interactor+renderers  from vtk
  /// 
  /// A frame contains a grid of subframes. Each subframe can contain
  /// several overlapping figures.
  class Frame
  {
  public:



    ///
    /// Create frame.
    ///
    Frame();
    
    
    ///
    /// Create smart pointer to frame.
    ///
    /// \return Smart pointer to frame object
    static std::shared_ptr<Frame> New()  { return std::make_shared<Frame>();}

    ///
    /// Destroy frame.
    ///
    ~Frame();

    ///
    /// Set layout of subframe grid
    ///
    /// \param nvpx  number of  rows
    /// \param nvpy  number of  columns
    void SetLayout(int nvpx, int nvpy);

    
    ///
    /// Write png image of frame content.
    /// 
    /// \param fname Image file name
    void WritePNG(std::string fname);


    ///
    /// Start video recording.
    /// 
    /// \param fname Video file name
    void StartVideo(std::string fname);

    
    ///
    /// Stop video recording.
    /// 
    void StopVideo();

    /// 
    /// Add figure at position (0)
    /// 
    void AddFigure(Figure *figure) {AddFigure(figure, 0);};

    /// 
    /// Add figure at position (0)
    /// 
    void AddFigure(Figure &figure) {AddFigure(&figure, 0);};

    /// 
    /// Add figure at position (0)
    /// 
    void AddFigure(std::shared_ptr<Figure> figure) {AddFigure(figure.get(),0);}


    /// 
    /// Add figure at position in frame
    /// 
    /// \param figure Figure to be added
    /// \param ipos Number of position in frame grid
    ///
    void AddFigure(Figure *figure, int ipos);

    /// 
    /// Add figure at position in frame
    /// 
    /// \param figure Figure to be added
    /// \param ipos Number of position in frame grid
    ///
    void AddFigure(Figure &figure, int ipos) {AddFigure(&figure,ipos);};

    /// 
    /// Add figure at position in frame
    /// 
    /// \param figure Figure to be added
    /// \param ipos Number of position in frame grid
    ///
    void AddFigure(std::shared_ptr<Figure> figure, int ipos){AddFigure(figure.get(),ipos);};


    /// 
    /// Remove figure
    /// 
    void RemoveFigure(Figure *figure);

    /// 
    /// Remove figure
    /// 
    void RemoveFigure(Figure &figure) {RemoveFigure(&figure);};

    /// 
    /// Remove figure
    /// 
    void RemoveFigure(std::shared_ptr<Figure> figure) {RemoveFigure(figure.get());}



    /// Link camera to other camera
    ///
    /// \param ivp  number of viewport
    /// \param frame other frame to be linked with 
    /// \param livp  number of link viewport
    void LinkCamera(int ivp,Frame& frame, int livp);

    /// Link camera to other camera
    ///
    /// \param ivp  number of viewport
    /// \param frame other frame to be linked with 
    /// \param livp  number of link viewport
    void LinkCamera(int ivp, std::shared_ptr<Frame> frame, int livp) {LinkCamera(ivp,*frame,livp);}

    /// Link camera to other camera
    ///
    /// \param ivp  number of viewport
    /// \param frame other frame to be linked with 
    /// \param livp  number of link viewport
    void LinkCamera(int ivp, Frame*frame, int livp) {LinkCamera(ivp, *frame,livp);}

    /// Link camera to other frame at default positions
    ///
    /// \param frame other frame to be linked with 
    void LinkCamera(std::shared_ptr<Frame> frame)  {LinkCamera(0,*frame,0);}

    /// Link camera to other frame at default positions
    ///
    /// \param frame other frame to be linked with 
    void LinkCamera(Frame& frame)  {LinkCamera(0,frame,0);}

    /// Set frame size in pixels
    ///
    /// \param x  Pixels in x direction
    /// \param y  Pixels in y direction
    void SetSize(int x, int y);

    ///
    /// Set window title
    /// 
    /// This title will show up in the title bar of the window.
    /// 
    /// \param title Title
    void SetWindowTitle(const std::string title);

    ///
    ///  Set Frame title
    /// 
    /// This title will show up within the frame
    ///
    ///  \param title Title

    void SetFrameTitle(const std::string title);

    ///
    /// Set frame position on screen
    ///
    /// \param x  x origin in pixels
    /// \param y  y origin in pixels
    void SetPosition(int x, int y);


    ///
    /// Toggle single/multi view for frame with mutiple figures
    ///
    void SetSingleSubFrameView(bool b);

    ///
    /// Set active subframe  number for editing, single/multiview etc.
    ///
    void SetActiveSubFrame(int iframe);

    ///
    ///  Set view angle for camera in active frame relative to default
    ///
    void SetActiveSubFrameCameraViewAngle(double angle);

    ///
    ///  Set focal point for camera in active frame relative to default
    ///
    void SetActiveSubFrameCameraFocalPoint(double x, double y, double z);

    ///
    ///  Set view position for camera in active frame relative to default
    ///
    void SetActiveSubFrameCameraPosition(double x, double y, double z);

    
    ///
    /// Show frame content and continue 
    ///
    /// Without interaction, this triggers a non-blocking run
    /// of the event loop. It can be blocked however by the space key.
    ///
    void Show();

    ///
    /// Show frame content and wait for interaction. 
    ///
    /// This performs a blocking run
    /// of the event loop. It can be unblocked however by the space key.
    ///
    void Interact();

    int GetStepNumber() { return step_number;}
    void SetStepNumber(int n) { step_number=std::max(n,0);}


    vtkSmartPointer<vtkRenderer> GetRenderer(Figure & fig);


    ///
    ///  Help string printed when pressing "h"/"?"
    ///
    static constexpr const char* KeyboardHelp=
R"(
--------------------------------------
   Key    Realm    Action

    Space Frame    Block/unblock calculation, increase step number
BackSpace Frame    Block/unblock calculation, decrease step number
   Escape Figure   Finish plane/level editing
   Return Figure   Store edited plane/level value and create new one
   Delete Figure   Delete last plane/level value
    Prior Frame    Show previous figure in single figure mode
    Next  Frame    Show next figure in single figure mode
        a Figure   Start arrow scale editing
        h Frame    Print this help to standard output
        l Figure   Start isolevel editing
        p Frame    Write frame to png
        q Frame    Abort
        r Figure   Reset camera (must left click to see the result)
        x Figure   Start x plane editing (3D only)
        y Figure   Start y plane editing (3D only)
        z Figure   Start z plane editing (3D only)
        A Figure   Toggle axis+box visibility
        B Figure   Toggle boundary cell visibility in vtkfig::GridView
        C Figure   Toggle interior cell visibility in vtkfig::GridView
        E Figure   Toggle elevation view (2D only)
        I Figure   Toggle isosurface view (3D only)
        L Figure   Generate 11 equally spaced isolevels.
        O Figure   Toggle outline visibility
        S Figure   Toggle surface visibility
        W Figure   Toggle wireframe mode 
        * Frame    Toggle single/multi figure view
        ? Frame    Print this help to standard output
        / Figure   Switch signs of cut plane

In  edit   mode,  left  mouse,   as  well  as  cursor   keys  increase
resp. decrease the value of the plane/isolevel/arrowscale, respectively.

Figures must be first clicked on before editing works.
--------------------------------------
)";

  private:

    friend class internals::MainThread;
    friend class internals::MyInteractorStyle;
    friend class internals::MyTimerCallback;
    friend class internals::Client;

    int step_number=0;

    /// Number of this frame in global frame list
    int number_in_frame_list=-1;

    /// List of all figures in frame
    std::set<Figure*>figures;

    /// Number of viewports in x direction
    int nvpx;

    /// Number of viewports in y direction
    int nvpy;

    struct SubFrame;

    void RTSetLayout(int nvpx, int nvpy);
    void RTCalculateViewports(int nx, int ny);
    void RTSetSingleViewport(int nx, int ny);
    void RTResetRenderers(bool from_scratch);
    void RTResetCamera(SubFrame& sf);

    void RTSetActiveSubFrameCameraViewAngle(SubFrame& sf, double a);
    void RTSetActiveSubFrameCameraFocalPoint(SubFrame& sf, double a[3]);
    void RTSetActiveSubFrameCameraPosition(SubFrame& sf, double a[3]);


    void RTAddFigures();
    void RTRemoveFigure(Figure *figure);
    void RTHideSubframe(SubFrame &subframe);
    void RTUnHideSubframe(SubFrame &subframe);
    void RTSetSingleView(bool single_viewport);
    void RTSetActiveSubFrame(int isub, bool hide_old);
    void SetAutoLayout(int nfig);


    /// Data structure decribing subframe
    /// Each subframe can hold several figures
    struct SubFrame
    {
      SubFrame(){};

      SubFrame(const double vp[4]):viewport{vp[0],vp[1],vp[2],vp[3]}{};
      

      /// vtkRenderer
      vtkSmartPointer<vtkRenderer>    renderer;

      /// Viewport within frame
      double viewport[4]={0,0,1,1};

      bool hidden=false;
    };
    bool single_subframe_view=false;
    int active_subframe=0;
    
    /// List of subframes
    std::vector<SubFrame> subframes;
    SubFrame title_subframe;

    /// Subframe position algebra
    int pos(const int ivpx, const int ivpy) { return (nvpy-ivpy-1)*nvpx+ivpx;}
    int ivpx(const int pos) { return pos%nvpx;}
    int ivpy(const int pos) { return nvpy-pos/nvpx-1;}


    /// Default camera data
    const double default_camera_focal_point[3]={0.65,0.5,0};
    const double default_camera_position[3]={0.65,0.5,10};
    const double default_camera_view_angle={15};
    // double default_camera_zoom={1};


    /// Parameters to be passed between threads
    struct
    {
      std::string filename; 
      std::string wintitle="vtkfig";
      std::string frametitle="";
      
      /// window sizes
      int winsize_x=400;
      int winsize_y=400;
      
      int winposition_x=0;
      int winposition_y=0;
      
      double camera_view_angle=15;
      double camera_focal_point[3]={0.65,0.5,0};
      double camera_position[3]={0.65,0.5,10};

      int camlinkthisframepos;
      int camlinkframepos;
      int camlinkframenum;
      
      int nvpx;
      int nvpy;
      
      int single_subframe_view;
      int active_subframe;

      Figure * current_figure;
    } parameter;
    /// The spinning main thread
    internals::MainThread *mainthread;
    
    /// Blocking send of commands to main thread
    void SendCommand(std::string source, internals::Communicator::Command cmd);
    
    /// Window vontaining frame
    vtkSmartPointer<vtkRenderWindow> window;
    
    /// Actor for frame title annotation
    vtkSmartPointer<vtkCornerAnnotation> title_actor=0;

    /// Init title actor
    void RTInit();

    /// Video writer
    vtkSmartPointer<vtkOggTheoraWriter> videowriter=0;


  };
}

#endif

