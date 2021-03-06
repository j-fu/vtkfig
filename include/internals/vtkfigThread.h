/**
    \file vtkfigThread.h

*/

#ifndef VTKFIG_THREAD_H
#define VTKFIG_THREAD_H

#include <memory>
#include <mutex>
#include <thread>
#include <memory>
#include <map>
#include <condition_variable>

#if USE_QT
#include <QVTKInteractor.h>
using Interactor=QVTKInteractor;
#else
#include <vtkRenderWindowInteractor.h>
using Interactor=vtkRenderWindowInteractor;
#endif

#include "vtkfigCommunicator.h"

namespace vtkfig
{
  class Frame;

  namespace internals
  {

    ///
    /// Main communication thread.
    ///
    /// Not to be exposed to user.
    class  Thread
    {
      friend class vtkfig::Frame;
      friend class Client;
      friend class TimerCallback;
      friend class InteractorStyle;

    public:
      ~Thread();
      Thread();
      
      /// Find frame with number
      static Frame& FindFrame(int number_in_framelist);


    private:
       vtkSmartPointer<Interactor> interactor;


      /// Start thread.
      ///
      /// Create a singleton object. We need only
      /// as vtk is not thread save.
      static std::shared_ptr<Thread> CreateThread();

      /// "This" thread
      static std::shared_ptr<Thread> mainthread;
      
      /// Kill the thread
      static void KillThread();

      /// Show all frames aka nonblocking event loop
      void Show();
      
      /// Blocking event loop demanding user interaction
      /// unblocked by space key
      void Interact();
      
      /// Add frame to be handeled by thread
      void AddFrame(std::shared_ptr<Frame> frame);
      
      /// Remove frame from thread
      void RemoveFrame(int number_in_framelist);


      /// Spawn parallel thread
      /// for rendering or communication
      void Start();
      
      /// Update all figures in all frames
      void Update();

      /// Terminate thread
      void Terminate(void);

      /// Map of all frames handeled by thread
      std::map<int,std::shared_ptr<Frame>> framemap;

      /// Prepare render thread before start
      static void PrepareRenderThread(Thread& thd);

      /// Callback function for render thread
      static void RenderThreadCallback(Thread& thd);

      /// Prepare comm thread before start
      static void PrepareCommunicatorThread(Thread& thd);

      /// Callback function for communicator thread
      static void CommunicatorThread(Thread& thd);

      /// Callback function for communicator thread 
      /// if multithreading is off
      static void CommunicatorThreadCallback(Thread& thd);

      
      /// Add frame, to be called from render thread
      static void RTAddFrame(Thread& mt, int iframe);

      /// Number of last frame created
      int lastframenum=0;

      /// std::thread which runs rendering resp. communication
      std::shared_ptr<std::thread> thread;


      /// Use of double buffering
      bool double_buffering=true;

      /// Timer interval (in ms) for timer callback
      /// This is a bit critica: if too small, the spinning process will eat lot of CPU.
      /// If too large, we will observe sluggish reaction.
      double timer_interval=20;

      /// Mutex to organize communication
      std::mutex mutex; 

      /// Condition variable signalizing finished command
      std::condition_variable condition_variable; 
        
      /// Thread state
      bool running_multithreaded=false;

      /// Toggle multithreaded/single (at startup)
      bool try_running_multithreaded=true; 
    
      /// space down state ?
      bool communication_blocked=false;


      /// Communication command
      Communicator::Command cmd=Communicator::Command::Empty; 


      /// Frame number as parameter during communication
      int iframe=-1;


      /// Communication with render thread
      void SendCommand(int iframe, const std::string from,Communicator::Command cmd);

      /// State of connection to client
      bool connection_open=false;
      
      /// Open connection to client
      void OpenConnection(int port, int wtime);

      /// Communicator object  for communication with cliend
      vtkSmartPointer<Communicator> communicator;
      
      /// Debug level for client communication
      int debug_level=0;


    };
  }
}

#endif
