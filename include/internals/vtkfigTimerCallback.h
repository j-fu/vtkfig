/**
    \file vtkfigTimerCallback.h
*/

#ifndef VTKFIG_TIMER_CALLBACK_H
#define VTKFIG_TIMER_CALLBACK_H

#include "vtkfigFrame.h"
#include "vtkfigFigure.h"
#include "vtkfigThread.h"

namespace vtkfig
{
  namespace internals
  {
///////////////////////////////////////////////////////////////////
    /// Timer callback handling communication with render thread
    class TimerCallback : public vtkCommand
    {

      TimerCallback(): vtkCommand(){};
    public:
      
      // Main thread to interact with
      Thread* mainthread;
            
      vtkSmartPointer<vtkRenderWindowInteractor> Interactor=0;
    
      static TimerCallback *New()    {return new TimerCallback();};
    
      
      virtual void Execute( vtkObject *vtkNotUsed(caller), unsigned long eventId, void *vtkNotUsed(callData));
    };
    
  }
}
#endif
