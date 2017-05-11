///
/// \file vtkfigFrame.h
///
/// Define Frame class  providing a window for rendering.
/// 

#ifndef VTKFIG_FRAME_H
#define VTKFIG_FRAME_H

#include <memory>
#include <mutex>
#include <thread>
#include <memory>
#include <vector>
#include <map>
#include <condition_variable>

#include "vtkSmartPointer.h"
#include "vtkRenderer.h"

#include "vtkfigCommunicator.h"
#include "vtkfigServerConnection.h"

namespace vtkfig
{

  class Figure;
  ///
  /// Frame: provide a thread based rendering
  /// 
  class Frame
  {
  public:

    ///
    /// Create a empty frame for local rendering
    ///
    static std::shared_ptr<Frame> New() { return std::make_shared<Frame>(); }
    static std::shared_ptr<Frame> New(int nrow, int ncol) { return std::make_shared<Frame>(nrow,ncol); }

    ///
    /// Create an empty frame  for server based rendering
    ///
    static std::shared_ptr<Frame> New(vtkSmartPointer<Communicator>comm, int nrow, int ncol) { return std::make_shared<Frame>(comm, nrow,  ncol); }

    static std::shared_ptr<Frame> New(ServerConnection& sconn, int nrow, int ncol) 
    {
      if (sconn.IsOpen())
        return New(sconn.GetCommunicator(),nrow,ncol); 
      else
        return New(nrow,ncol); 
    }

    static std::shared_ptr<Frame> New(ServerConnection& sconn) 
    {
      return New(sconn,1,1);
    }
    static std::shared_ptr<Frame> New(std::shared_ptr<ServerConnection> sconn) 
    { return New(*sconn);}
    
    Frame(): Frame(1,1){};

    Frame(vtkSmartPointer<Communicator>comm): Frame(comm,1,1){};

    Frame(const int nrow, const int ncol);

    Frame(vtkSmartPointer<Communicator>, const int nrow, const int ncol);

    ~Frame();
    
    void Dump(std::string fname);
        
    void AddFigure(std::shared_ptr<Figure> figure, int irow, int icol);

    void AddFigure(std::shared_ptr<Figure> figure) {AddFigure(figure,0,0);}

    void Show();

    void Interact();

    void Resize(int x, int y);

    void Reposition(int x, int y);

    std::vector<std::shared_ptr<Figure>> figures;
    
    static Frame* frame(int key) { return frames[key];}

  private:


    static std::map<int, Frame*> frames;

    static int lastframenum;

    int framenum=0;



    static constexpr const char* keyboard_help=
R"(
--------------------------------------
Key     Action

space  Interrupt/continue calculation
r      Reset camera
w      Wireframe modus
--------------------------------------
)";

    void RestartRenderThread(void);
    void StartCommunicatorThread(void);
    void StartRenderThread(void);
    void Terminate(void);

    const int nrow;

    const int ncol;

    enum class Command
    {
      None=0,
        Show,
        Dump,            
        Resize,            
        AddFigure,            
        Reposition,
        Clear,            
        Terminate,
        SetBackgroundColor          
    };

    /// Communication command
    Command cmd; 

    void SendCommand(const std::string from, Command cmd);


    /// mutex to organize communication
    std::mutex mtx; 

    /// condition variable signalizing finished command
    std::condition_variable cv; 
    
    /// File name to be passed 
    std::string fname; 

    /// window sizes
    int win_x=400;
    int win_y=400;
    
    /// Thread state
    bool thread_alive=false;
    
    /// space down state ?
    bool communication_blocked=false;
    
    /// 
    bool wireframe;

    /// 
    struct SubFrame
    {
      SubFrame(){};
      SubFrame(const double vp[4]):viewport{vp[0],vp[1],vp[2],vp[3]}{};
      double default_camera_focal_point[3]={0,0,0};
      double default_camera_position[3]={0,0,20};
      vtkSmartPointer<vtkRenderer>    renderer;
      double viewport[4]={0,0,1,1};
    };

    vtkSmartPointer<vtkfig::Communicator> communicator=0;
    std::shared_ptr<std::thread> thread;

    /// Each subframe can hold several figures

    std::vector<SubFrame> subframes;

    
    int pos(const int irow, const int icol) { return irow*ncol+icol;}
    int row(const int pos) { return pos/ncol;}
    int col(const int pos) { return pos%ncol;}

    friend class  InteractorStyleTrackballCamera;
    friend class  TimerCallback;
    static void RenderThread(Frame* frame);
    static void CommunicatorThread(Frame* frame);
  };
}

#endif

