#include <cassert>

#include "vtkfigFrame.h"
#include "vtkfigFigure.h"
#include "vtkfigMainThread.h"

namespace vtkfig
{

  /////////////////////////////////////
  /// Public API 
  
  Frame::Frame(const int nrow, const int ncol):
  nrow(nrow),
    ncol(ncol)
  {
    figures.clear();
    double dx= 1.0/(double)ncol;
    double dy= 1.0/(double)nrow;
    subframes.resize(nrow*ncol);
    
    double y=1.0;
    for (int irow=0 ;irow<nrow;irow++, y-=dy)
    {
      double x=0.0;
      for (int icol=0;icol<ncol;icol++, x+=dx)
      {
        int ipos=pos(irow,icol);
        
        assert(row(ipos)==irow);
        assert(col(ipos)==icol);
        
        
        auto & subframe=subframes[ipos];
        subframe.viewport[0]=x;
        subframe.viewport[1]=y-dy;
        subframe.viewport[2]=x+dx;
        subframe.viewport[3]=y;
      }
    }
  }



  std::shared_ptr<Frame> Frame::New(int nrow, int ncol) 
  {
    auto frame=std::make_shared<Frame>(nrow,ncol);
    MainThread::CreateMainThread();
    MainThread::mainthread->AddFrame(frame);
    return frame;
  };
  

  void Frame::LinkCamera(int irow, int icol, std::shared_ptr<Frame> frame, int lirow, int licol)
  {
    auto & subframe= subframes[pos(irow,icol)];
    int linkframepos=frame->pos(lirow,licol);
    int linkframenum=frame->framenum;
    mainthread->LinkCamera(framenum,pos(irow,icol),frame->framenum,frame->pos(lirow,licol));
  }
  
  void Frame::Show() { mainthread->Show();}

  void Frame::AddFigure(std::shared_ptr<Figure> fig, int irow, int icol)
  {
    assert(irow<this->nrow);
    assert(icol<this->ncol);
    
    this->figures.push_back(fig);
    int pos=this->pos(irow,icol);
    fig->framepos=pos;
    
    SendCommand("AddFigure", Communicator::Command::FrameAddFigure);
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
  }
  
  
  // void Frame::Clear(void)
  // {
  //   this->figures.clear();
  //   SendCommand(Communicator::Command::Clear;
  //   std::unique_lock<std::mutex> lock(this->mtx);
  //   this->cv.wait(lock);
  // }




}
