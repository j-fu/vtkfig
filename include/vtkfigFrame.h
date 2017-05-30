///
/// \file vtkfigFrame.h
///
/// Define Frame class  providing a window for rendering.
/// 
#ifndef VTKFIG_FRAME_H
#define VTKFIG_FRAME_H

#include <memory>
#include <vector>

#include "vtkSmartPointer.h"
#include "vtkRenderer.h"
#include "vtkCornerAnnotation.h"
#include "vtkfigCommunicator.h"

namespace vtkfig
{

  class Figure;
  class MainThread;

  ///
  /// Provide Window+interactor+renderers 
  /// 
  class Frame
  {
  public:


    ///
    /// Create frame with multiple subframes 
    ///
    /// Viewports are arranged in a rectagular scheme
    ///
    /// \param nvpx  number of viewport columns
    /// \param nvpx  number of viewport rows
    ///
    Frame(const int nvpx, const int nvpy);

    ///
    /// Create frame with single subframe 
    ///
    Frame(): Frame(1,1){};
    
    static std::shared_ptr<Frame> New(int nvpx, int nvpy) 
    {
      return std::make_shared<Frame>(nvpx,nvpy);
    };

    ///
    /// Create frame with single subframe 
    ///
    static std::shared_ptr<Frame> New() {return  Frame::New(1,1);}

    ~Frame();
    
    void Dump(std::string fname);
        
    void AddFigure(Figure *figure) {AddFigure(figure, 0,0);};
    void AddFigure(Figure &figure) {AddFigure(&figure, 0,0);};
    void AddFigure(std::shared_ptr<Figure> figure) {AddFigure(figure.get(),0,0);}

    void AddFigure(Figure *figure, int ivpx, int ivpy);
    void AddFigure(Figure &figure, int ivpx, int ivpy) {AddFigure(&figure,ivpx, ivpy);};
    void AddFigure(std::shared_ptr<Figure> figure, int ivpx, int ivpy){AddFigure(figure.get(),ivpx,ivpy);};

    void AddFigure(Figure *figure, int ipos);
    void AddFigure(Figure &figure, int ipos) {AddFigure(&figure,ipos);};
    void AddFigure(std::shared_ptr<Figure> figure, int ipos){AddFigure(figure.get(),ipos);};


    void LinkCamera(int ivpx, int ivpy,Frame& frame, int livpx, int livpy);
    void LinkCamera(Frame& frame)  {LinkCamera(0,0,frame,0,0);}
    void LinkCamera(int ivpx, int ivpy,std::shared_ptr<Frame> frame, int livpx, int livpy) {LinkCamera(ivpx,ivpy,*frame,livpx,livpy);}
    void LinkCamera(int ivpx, int ivpy, Frame*frame, int livpx, int livpy) {LinkCamera(ivpx,ivpy,*frame,livpx,livpy);}
    void LinkCamera(std::shared_ptr<Frame> frame)  {LinkCamera(0,0,*frame,0,0);}


    void SetSize(int x, int y);

    void SetWindowTitle(const std::string title);

    void SetFrameTitle(const std::string title);

    void SetPosition(int x, int y);

    std::vector<Figure*>figures;
    
    void Show();

    void Interact();


    int framenum=-1;



    static constexpr const char* keyboard_help=
R"(
--------------------------------------
   Key     Action

    Space  Interrupt/continue calculation
   Escape  Finish plane/level editing
   Return  Store edited plane/level value
BackSpace  Delete last plane/level value
        l  Start isolevel editing
        L  Generate 11 equally spaced isolevels.
        i  Toggle isosurface view (3D only)
        r  Reset camera
        w  Toggle wireframe mode 
        x  Start x plane editing (3D only)
        y  Start y plane editing (3D only)
        z  Start z plane editing (3D only)

In edit mode, left mouse, as well as
cursor keys move value of plane/isolevel.
--------------------------------------
)";

    const int nvpx;

    const int nvpy;


    /// File name to be passed 
    std::string fname; 

    /// window sizes
    int win_x=400;
    int win_y=400;
    
    int pos_x=0;
    int pos_y=0;

    std::string wintitle;
    std::string frametitle;

    int camlinkthisframepos;
    int camlinkframepos;
    int camlinkframenum;

    vtkSmartPointer<vtkCornerAnnotation> title_actor;
    
    /// 
    struct SubFrame
    {
      SubFrame(){};
      SubFrame(const double vp[4]):viewport{vp[0],vp[1],vp[2],vp[3]}{};
      double default_camera_focal_point[3]={0.65,0.5,0};
      double default_camera_position[3]={0.65,0.5,10};
      double default_camera_zoom={1};
      double default_camera_view_angle={15};
      double panscale=1.0;
      vtkSmartPointer<vtkRenderer>    renderer;
      double viewport[4]={0,0,1,1};
    };

    MainThread *mainthread;
    
    void SendCommand(std::string source, Communicator::Command cmd);
    /// Each subframe can hold several figures

    std::vector<SubFrame> subframes;

    vtkSmartPointer<vtkRenderWindow> window;
    
    int pos(const int ivpx, const int ivpy) { return (nvpy-ivpy-1)*nvpx+ivpx;}
    int ivpx(const int pos) { return pos%nvpx;}
    int ivpy(const int pos) { return nvpy-pos/nvpx-1;}

    friend class  InteractorStyleTrackballCamera;
    friend class  TimerCallback;


  };
}

#endif

