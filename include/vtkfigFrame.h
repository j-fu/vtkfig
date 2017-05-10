///
/// \file vtkfigFrame.h
///
/// Define Frame class  providing a window for rendering.
/// 


#ifndef VTKFIG_FRAME_H
#define VTKFIG_FRAME_H

#include <memory>
#include "vtkSmartPointer.h"
#include "vtkfigCommunicator.h"
#include "vtkfigServerConnection.h"

namespace vtkfig
{

  class Figure;
  class FrameContent;

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
    
  private:
    bool server_mode=false;
    void Restart(void);
    void Start(void);
    void Terminate(void);
    vtkSmartPointer<FrameContent> framecontent;
  };
}

#endif

