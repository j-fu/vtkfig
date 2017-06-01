#include <cassert>

#include "vtkTextProperty.h"

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
    double dy= 0.925/(double)nvpy;
    double dx= 1.0/(double)nvpx;
    subframes.resize(nvpx*nvpy+1);
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
    auto & subframe=subframes[nvpx*nvpy];
    subframe.viewport[0]=0;
    subframe.viewport[1]=0.925;
    subframe.viewport[2]=1.0;
    subframe.viewport[3]=1.0;

    title_actor= vtkSmartPointer<vtkCornerAnnotation>::New();
    auto textprop=title_actor->GetTextProperty();
    textprop->ItalicOff();
    textprop->BoldOff();
    textprop->SetFontSize(12);
    textprop->SetFontFamilyToArial();
    textprop->SetColor(0,0,0);
    

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
  
  void Frame::AddFigure(Figure* fig, int ipos)
  {
    AddFigure(fig,ivpx(ipos),ivpy(ipos));
  }


  void Frame::LinkCamera(int ivpx, int ivpy, Frame& frame, int livpx, int livpy)
  {
    parameter.camlinkthisframepos=pos(ivpx,ivpy);
    parameter.camlinkframepos=frame.pos(livpx,livpy);
    parameter.camlinkframenum=frame.number_in_frame_list;
    SendCommand("LinkCamera", Communicator::Command::FrameLinkCamera);
  }
  

  void Frame::WritePNG(std::string fname)
  {
    parameter.filename=fname;
    SendCommand("Dump", Communicator::Command::FrameDump);
  }

  void Frame::SetSize(int x, int y)
  {
    parameter.winsize_x=x;
    parameter.winsize_y=y;
    SendCommand("Size", Communicator::Command::FrameSize);
  }

  void Frame::SetPosition(int x, int y)
  {
    parameter.winposition_x=x;
    parameter.winposition_y=y;
    SendCommand("Position", Communicator::Command::FramePosition);
  }

  void Frame::SetWindowTitle(const std::string title)
  {
    parameter.wintitle=title;
    SendCommand("WindowTitle", Communicator::Command::WindowTitle);
  }

  void Frame::SetFrameTitle(const std::string title)
  {
    parameter.frametitle=title;
    SendCommand("FrameTitle", Communicator::Command::FrameTitle);
  }


  void Frame::SendCommand(std::string source, Communicator::Command comm)
  {
    mainthread->SendCommand(number_in_frame_list, source, comm);
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
