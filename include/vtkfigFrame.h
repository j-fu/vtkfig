#ifndef VTKFIG_FRAME_H
#define VTKFIG_FRAME_H

#include <memory>
#include <thread>

#include "vtkSmartPointer.h"


namespace vtkfig
{

  class Figure;
  class Communicator;
  
  class Frame
  {
  public:
    
    enum class InteractorStyle
    {
      Planar=2,
        Volumetric
        };
    

    Frame();

    ~Frame();
    
    void Dump(std::string fname);
    
    void Clear(void);
    
    void Add(Figure & figure);

    void Show();

    void Interact();
    
    void SetInteractorStyle(InteractorStyle style);
    
  private:
    void Restart(void);
    void Start(void);
    void Terminate(void);
    vtkSmartPointer<Communicator> communicator;
    std::shared_ptr<std::thread> render_thread;
  };
}

#endif

