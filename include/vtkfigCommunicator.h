#ifndef VTKFIG_COMMUNICATOR_H
#define VTKFIG_COMMUNICATOR_H

#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>


#include "vtkObjectBase.h"
#include "vtkProp.h"

#include "vtkfigFrame.h"

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
    
    /// Actors to be passed
    std::shared_ptr<std::vector<vtkSmartPointer<vtkProp>>> actors; 
    
    /// Thread state
    bool render_thread_alive=false;
    
    /// space down state ?
    bool communication_blocked=false;
    
    /// interactor style
    Frame::InteractorStyle interactor_style= Frame::InteractorStyle::Planar;
    
    /// backgroud color
    double bgcolor[3]={1,1,1};

    Communicator();

    Communicator(const Communicator& A)=delete;
    static Communicator *New();

  };
}
#endif

