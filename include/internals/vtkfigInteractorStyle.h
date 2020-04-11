/**
    \file vtkfigInteratorStyle.h
*/

#ifndef VTKFIG_INTERACTOR_STYLE_H
#define VTKFIG_INTERACTOR_STYLE_H

#include <chrono>
#include <vtkInteractorStyleTrackballCamera.h>
#include "vtkfigFrame.h"
#include "vtkfigFigure.h"

namespace vtkfig
{
  namespace internals
  {
    
    ///
    ///  vtkfig specific keybord and mouse interaction
    ///
    class InteractorStyle : public vtkInteractorStyleTrackballCamera
    {
      
      /// List of currently edited figures
      std::vector<std::shared_ptr<Figure> >edited_figures;
      
      /// Figure edit mode
      bool edit_mode =false;

      /// Mouse button down 
      bool left_button_down=false;

      /// Last mouse  x position
      int lastx=0;

      /// Last mouse  y position
      int lasty=0;
      
      /// Current frame
      Frame* frame;

      
    public:

      
    
      static InteractorStyle* New() {return new InteractorStyle();}
      InteractorStyle(): vtkInteractorStyleTrackballCamera(){};

      void SetFrame(Frame* xframe);

      virtual void OnConfigure();
      
      /// Overwrite left button down
      virtual void OnLeftButtonDown();

      /// Overwrite left button up
      virtual void OnLeftButtonUp();

      /// Overwrite  mouse move
      virtual void OnMouseMove();

      ///  Overwrite keyboard callback
      virtual void OnChar();
    };
  }
}

#endif
