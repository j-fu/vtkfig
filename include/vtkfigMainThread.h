/**
    \file vtkfigMainThread.h

    Namespace vtkfig::internals: Tools for server-client communication.
*/

#include <memory>
#include <mutex>
#include <thread>
#include <memory>
#include <map>
#include <condition_variable>

#include <vtkRenderWindowInteractor.h>

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
    class  MainThread
    {
      friend class vtkfig::Frame;
      friend class Client;
      friend class MyTimerCallback;
      friend class MyInteractorStyle;

    private:
      ~MainThread();
      MainThread();
      vtkSmartPointer<vtkRenderWindowInteractor> interactor;


      /// Start thread.
      ///
      /// Create a singleton object. We need only
      /// as vtk is not thread save.
      static MainThread * CreateMainThread();

      /// "This" thread
      static MainThread *mainthread;
      
      /// Kill the thread
      static void DeleteMainThread();

      /// Show all frames aka nonblocking event loop
      void Show();
      
      /// Blocking event loop demanding user interaction
      /// unblocked by space key
      void Interact();
      
      /// Add frame to be handeled by thread
      void AddFrame(Frame *frame);
      
      /// Remove frame from thread
      void RemoveFrame(Frame *frame);

      /// Spawn parallel thread
      /// for rendering or communication
      void Start();
      
      /// Update all figures in all frames
      void Update();

      /// Terminate thread
      void Terminate(void);

      /// Map of all frames handeled by thread
      std::map<int,Frame*> framemap;

      /// Prepare render thread before start
      static void PrepareRenderThread(MainThread*);

      /// Callback function for render thread
      static void RenderThread(MainThread*);

      /// Prepare comm thread before start
      static void PrepareCommunicatorThread(MainThread*);

      /// Callback function for communicator thread
      static void CommunicatorThread(MainThread*);

      /// Callback function for communicator thread 
      /// if multithreading is off
      static void CommunicatorThreadCallback(MainThread*);

      
      /// Add frame, to be called from render thread
      static void RTAddFrame(MainThread* mt, int iframe);

      /// Number of last frame created
      int lastframenum=0;

      /// std::thread which runs rendering resp. communication
      std::shared_ptr<std::thread> thread;


      /// Use of double buffering
      bool double_buffering=true;

      /// Timer interval for timer callback
      double timer_interval=10;

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
