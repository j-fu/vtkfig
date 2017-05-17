#include <cassert>

#include "vtkfigFrame.h"
#include "vtkfigFigure.h"
#include "vtkfigMainThread.h"

namespace vtkfig
{

  /////////////////////////////////////
  /// Public API 
  
  Frame::Frame(const int nvpx, const int nvpy):
    nvpx(nvpx),
    nvpy(nvpy)
  {
    MainThread::CreateMainThread();

    figures.clear();
    double dy= 1.0/(double)nvpy;
    double dx= 1.0/(double)nvpx;
    subframes.resize(nvpx*nvpy);
    double y=0.0;
    for (int ivpy=0;ivpy<nvpy;ivpy++, y+=dy)
    {
      double x=0.0;
      for (int ivpx=0 ;ivpx<nvpx;ivpx++, x+=dx)
      {
        int ipos=pos(ivpx,ivpy);
        
        assert(this->ivpx(ipos)==ivpx);
        assert(this->ivpy(ipos)==ivpy);
        
        auto & subframe=subframes[ipos];
        subframe.viewport[0]=x;
        subframe.viewport[1]=y;
        subframe.viewport[2]=x+dx;
        subframe.viewport[3]=y+dy;
      }
    }
    MainThread::mainthread->AddFrame(this);

  }
  

  
  void Frame::Show() { mainthread->Show();}

  void Frame::Interact() { mainthread->Interact();}

  void Frame::AddFigure(Figure* fig, int ivpx, int ivpy)
  {
    assert(ivpx<this->nvpx);
    assert(ivpy<this->nvpy);
    this->figures.push_back(fig);
    fig->framepos=this->pos(ivpx,ivpy);
    SendCommand("AddFigure", Communicator::Command::FrameAddFigure);
  }
  

  void Frame::LinkCamera(int ivpx, int ivpy, Frame& frame, int livpx, int livpy)
  {
    camlinkthisframepos=pos(ivpx,ivpy);
    camlinkframepos=frame.pos(livpx,livpy);
    camlinkframenum=frame.framenum;
    SendCommand("LinkCamera", Communicator::Command::FrameLinkCamera);
  }
  

  void Frame::Dump(std::string fname)
  {
    this->fname=fname;
    SendCommand("Dump", Communicator::Command::FrameDump);
  }

  void Frame::Size(int x, int y)
  {
    this->win_x=x;
    this->win_y=y;
    SendCommand("Size", Communicator::Command::FrameSize);
  }

  void Frame::Position(int x, int y)
  {
    this->pos_x=x;
    this->pos_y=y;
    SendCommand("Position", Communicator::Command::FramePosition);
  }

  void Frame::SendCommand(std::string source, Communicator::Command comm)
  {
    mainthread->SendCommand(framenum, source, comm);
  }

  Frame::~Frame()
  {
    MainThread::mainthread->RemoveFrame(this);
    
  }
  
  
  // void Frame::Clear(void)
  // {
  //   this->figures.clear();
  //   SendCommand(Communicator::Command::Clear;
  //   std::unique_lock<std::mutex> lock(this->mtx);
  //   this->cv.wait(lock);
  // }




}
