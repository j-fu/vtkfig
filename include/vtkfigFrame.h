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
  


  class Frame
  {
  public:
    
    enum class InteractorStyle
    {
      Planar=2,
        Volumetric
        };
    
    static std::shared_ptr<Frame> New() { return std::make_shared<Frame>(); }
    static std::shared_ptr<Frame> New(vtkSmartPointer<Communicator>comm) { return std::make_shared<Frame>(comm); }
    static std::shared_ptr<Frame> New(ServerConnection& sconn) 
    {
      if (sconn.IsOpen())
        return New(sconn.GetCommunicator()); 
      else
        return New(); 
    }
    static std::shared_ptr<Frame> New(std::shared_ptr<ServerConnection> sconn) { return New(*sconn);}
    
    Frame();
    Frame(vtkSmartPointer<Communicator>);
    Frame(int nrow, int ncol);

    ~Frame();
    
    void Dump(std::string fname);
    
    void Clear(void);
    
    void AddFigure(std::shared_ptr<Figure> figure);

    void Show();

    void Interact();
    
    void SetInteractorStyle(InteractorStyle style);
    
  private:
    bool server_mode=false;
    void Restart(void);
    void Start(void);
    void Terminate(void);
    vtkSmartPointer<FrameContent> framecontent;
  };
}

#endif

