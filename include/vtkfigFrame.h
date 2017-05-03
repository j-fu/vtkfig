#ifndef VTKFIG_FRAME_H
#define VTKFIG_FRAME_H

#include <memory>
#include "vtkSmartPointer.h"

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
    
    Frame();

    ~Frame();
    
    void Dump(std::string fname);
    
    void Clear(void);
    
    void AddFigure(std::shared_ptr<Figure> figure);

    void Show();

    void Interact();
    
    void SetInteractorStyle(InteractorStyle style);
    
  private:
    void Restart(void);
    void Start(void);
    void Terminate(void);
    vtkSmartPointer<FrameContent> framecontent;
  };
}

#endif

