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
  

  void Frame::AddFigure(std::shared_ptr<Figure> fig, int irow, int icol)
  {
    assert(irow<this->nrow);
    assert(icol<this->ncol);
    
    this->figures.push_back(fig);
    int pos=this->pos(irow,icol);
    fig->framepos=pos;
    
    mainthread->SendCommand(framenum, "AddFigure", MainThread::Command::AddFigure);
  }
  
  

  void Frame::Dump(std::string fname)
  {
    this->fname=fname;
    if (mainthread)
      mainthread->SendCommand(framenum, "Dump", MainThread::Command::Dump);
  }

  void Frame::Resize(int x, int y)
  {
    this->win_x=x;
    this->win_y=y;
    if (mainthread)
      mainthread->SendCommand(framenum, "Resize", MainThread::Command::Resize);
  }

  void Frame::Reposition(int x, int y)
  {
    this->pos_x=x;
    this->pos_y=y;
    if (mainthread)
      mainthread->SendCommand(framenum, "Reposition", MainThread::Command::Reposition);
  }


  Frame::~Frame()
  {
    // ??? Remove from thread
  }
  
  
  // void Frame::Clear(void)
  // {
  //   this->figures.clear();
  //   SendCommand(MainThread::Command::Clear;
  //   std::unique_lock<std::mutex> lock(this->mtx);
  //   this->cv.wait(lock);
  // }




}
