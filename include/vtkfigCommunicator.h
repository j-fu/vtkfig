#ifndef VTKFIG_COMMUNICATOR_H
#define VTKFIG_COMMUNICATOR_H

#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>


#include "vtkObjectBase.h"
#include "vtkActor.h"
#include "vtkActor2D.h"

#include "vtkfigFrame.h"
#include "vtkfigFigure.h"

namespace vtkfig
{
  ////////////////////////////////////////////////
  /// Communicator class Frame-> RenderThread
 
  class Communicator: public vtkObjectBase
  {
  public:
    enum class Command
    {
      None=0,
        Show,
        Dump,            
        Clear,            
        Terminate,
        SetInteractorStyle,
        SetBackgroundColor          
    };

    /// Communication command
    Command cmd; 

    /// mutex to organize communication
    std::mutex mtx; 

    /// condition variable signalizing finished command
    std::condition_variable cv; 
    
    /// File name to be passed 
    std::string fname; 

    std::shared_ptr<std::vector<std::shared_ptr<Figure>>> figures;
    
    /// Thread state
    bool render_thread_alive=false;
    
    /// space down state ?
    bool communication_blocked=false;
    
    /// interactor style
    Frame::InteractorStyle interactor_style= Frame::InteractorStyle::Planar;

    Communicator();

    Communicator(const Communicator& A)=delete;
    static Communicator *New();

  };
}
#endif

