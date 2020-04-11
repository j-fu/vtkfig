#include <memory>
#include <mutex>
#include <thread>
#include <memory>
#include <map>
#include <condition_variable>
#include <vtkRenderWindowInteractor.h>


#include <stdexcept>
#ifdef __APPLE__
#include <unistd.h>
#endif

#include <vtkRenderWindow.h>

#include <vtkCommand.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkObjectBase.h>
#include <vtkPropCollection.h>

#include "config.h"

#include "vtkfigFrame.h"
#include "vtkfigFigure.h"

#include "internals/vtkfigCommunicator.h"
#include "internals/vtkfigInteractorStyle.h"
#include "internals/vtkfigTimerCallback.h"


/*
FindFrame + framemap -> frame module
  

 */

namespace vtkfig
{
  class Frame;
  namespace internals
  {

    class XXThread
    {
    public:
      /// Communication with render thread
      void SendCommand(int iframe, const std::string from,Communicator::Command cmd);

      /// Communication command
      Communicator::Command cmd=Communicator::Command::Empty; 


      /// Frame number as parameter during communication
      int iframe=-1;

      /// std::thread which runs rendering resp. communication
      std::shared_ptr<std::thread> thread;


      /// Mutex to organize communication
      std::mutex mutex; 

      /// Condition variable signalizing finished command
      std::condition_variable condition_variable; 

      /// Debug level for client communication
      int debug_level=0;

      static std::shared_ptr<XXThread> CreateXXRenderThread(Frame &frame);
      static std::shared_ptr<XXThread> CreateXXCommunicatorThread();
    };

    void XXThread::SendCommand(int number_in_frame_list, const std::string from, Communicator::Command cmd)
    {
      if (debug_level>0)
        cout << ": XXThread::SendCommand " << from << " " << number_in_frame_list << endl;
      
      this->cmd=cmd;
      this->iframe=number_in_frame_list;
      std::unique_lock<std::mutex> lock(this->mutex);
      this->condition_variable.wait(lock);
    }
    

    
    class XXRenderThread: public XXThread
    {
    public:

      Frame & frame;
      XXRenderThread(Frame & frame);

      
      vtkSmartPointer<vtkRenderWindowInteractor> interactor;
      /// Show all frames aka nonblocking event loop
      void Show();
      
      /// Blocking event loop demanding user interaction
      /// unblocked by space key
      void Interact();

      /// Spawn parallel thread
      /// for rendering or communication
      void Start();
      
      /// Update all figures in all frames
      void Update();

      /// Terminate thread
      void Terminate(void);

      /// Map of all frames handeled by thread
      std::map<int,std::shared_ptr<Frame>> framemap;

      /// Callback function for render thread
      static void ThreadMain(XXRenderThread& thd);

      void RTPrepare(void);

      /// Timer interval (in ms) for timer callback
      /// This is a bit critica: if too small, the spinning process will eat lot of CPU.
      /// If too large, we will observe sluggish reaction.
      double timer_interval=20;
      
      /// space down state ?
      bool communication_blocked=false;
    };

    std::shared_ptr<XXThread> XXThread::CreateXXRenderThread(Frame&frame)
    {
      auto* ptr = new XXRenderThread(frame);
      std::shared_ptr<XXThread> shptr{ptr};
      return shptr;
    }

    
    XXRenderThread::XXRenderThread(Frame & frame): frame(frame)
    {
      this->thread=std::make_shared<std::thread>(ThreadMain,std::ref(*this));
    }
    
    void XXRenderThread::RTPrepare(void)
    {
      frame.window = vtkSmartPointer<vtkRenderWindow>::New();
      frame.window->SetWindowName("");

      frame.window->SetSize(frame.parameter.winsize_x, frame.parameter.winsize_y);
      frame.window->SetPosition(frame.parameter.winposition_x, frame.parameter.winposition_y);
      
      frame.RTResetRenderers(true);

      frame.RTInit();
      frame.title_subframe.renderer->AddActor(frame.title_actor);
      frame.title_actor->SetText(6,"---");
      frame.title_actor->SetText(7,frame.parameter.frametitle.c_str());


      interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
      auto style =  vtkSmartPointer<InteractorStyle>::New();
      style->SetFrame(&frame);
      interactor->SetInteractorStyle(style);
    
      interactor->SetRenderWindow(frame.window);
      auto callback =  vtkSmartPointer<TimerCallback>::New();

      callback->Interactor=interactor;
      interactor->AddObserver(vtkCommand::TimerEvent,callback);

      interactor->Initialize();

      interactor->CreateRepeatingTimer(timer_interval);
    }


    void XXRenderThread::Show()
    {
      SendCommand(-1, "Show", Communicator::Command::RenderThreadShow);
    }


    void XXRenderThread::Interact()
    {
      
      SendCommand(-1, "ShowAndBlock", Communicator::Command::RenderThreadShowAndBlock);

      do
      {
        std::this_thread::sleep_for (std::chrono::milliseconds(10));
      }
      while (this->communication_blocked);

    }


  
    void XXRenderThread::ThreadMain(XXRenderThread& xmainthread)
    {
      xmainthread.RTPrepare();
      
      if (xmainthread.debug_level>0)
        cout << ": XRenderThread start" << endl;
      
      xmainthread.interactor->Start();
      if (xmainthread.debug_level>0)
        cout << ": XRenderThread prepare termination" << endl;
      
      for (auto & subframe: xmainthread.frame.subframes)
        subframe.renderer->RemoveAllViewProps();
      xmainthread.frame.window->Finalize();
      
      
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      xmainthread.condition_variable.notify_all();
      
      xmainthread.frame.window=0;
      
      xmainthread.interactor=0;
      
      if (xmainthread.debug_level>0)
        cout << ": XRenderThread stop" << endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }


    
    ///////////////////////////////////////////
    class XXCommunicatorThread: public XXThread
    {
    public: 
      XXCommunicatorThread();
      /// State of connection to client
      bool connection_open=false;
      
      /// Open connection to client
      void OpenConnection(int port, int wtime);

      /// Communicator object  for communication with cliend
      vtkSmartPointer<Communicator> communicator;

      /// Callback function for communicator thread
      static void ThreadMain(XXCommunicatorThread& thd);

      /// Callback function for communicator thread 
      /// if multithreading is off
      static void RTCallback(XXCommunicatorThread& thd);

    };

    std::shared_ptr<XXThread> XXThread::CreateXXCommunicatorThread()
    {
      auto* ptr = new XXCommunicatorThread();
      std::shared_ptr<XXThread> shptr{ptr};
      return shptr;
    }

    void XXCommunicatorThread::OpenConnection(int port, int wtime)
    {
      if (connection_open) return;

      cout << ": Server start listening on port "<< port << endl;
      communicator=vtkSmartPointer<Communicator>::New();
      communicator->server_listen_num_retry=1;
      communicator->server_listen_waiting_time=1000*wtime;
      int rc=communicator->ServerMPConnect(port);
      if (rc)  
        cout << ": Server connected" << endl;
      else
      {
        cout << ": Nobody is listening ... giving up" << endl;
        throw std::runtime_error("Server connection failed");
        exit(1);
      }
      connection_open=true;
    }

      
    XXCommunicatorThread::XXCommunicatorThread()
    {
      int wtime=5;
      char* port_string= getenv("_PORT_NUMBER");
      char* wtime_string=getenv("_WAIT_SECONDS");
      char *debug_string=getenv("_DEBUG");
      char *multi_string=getenv("_MULTITHREADED");
      char *dbuff_string=getenv("_DOUBLEBUFFER");



      if (debug_string!=0)
        debug_level=atoi(debug_string);

      
      if (wtime_string!=0)
        wtime=atoi(wtime_string);
    
    
      if (port_string!=0)
      {
        int port=atoi(port_string);
        if (port<=0) 
          throw std::runtime_error(": Invalid port number for server");
        OpenConnection(port,wtime);
      }

      if (debug_level>0)
      {   
        std::string sc="client";
        if (connection_open) 
          sc="server";
      }
      this->thread=std::make_shared<std::thread>(ThreadMain,std::ref(*this));
    }

    void  XXCommunicatorThread::ThreadMain(XXCommunicatorThread& xmainthread)
    {
//      PrepareCommunicatorThread(xmainthread);
      while(1)
      {
        std::this_thread::sleep_for (std::chrono::milliseconds(5));
        RTCallback(xmainthread);
      }
    }

    void  XXCommunicatorThread::RTCallback(XXCommunicatorThread& xmainthread)
    {
      if (xmainthread.cmd!=Communicator::Command::Empty)
      {      
        
        // Lock mutex
        std::unique_lock<std::mutex> lock(xmainthread.mutex);

//        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        if (xmainthread.debug_level>0) 
          cout << ": send cmd: " << static_cast<int>(xmainthread.cmd) << " frame: " <<xmainthread.iframe<< endl;


        xmainthread.communicator->SendCommand(xmainthread.cmd);
        xmainthread.communicator->SendInt(xmainthread.iframe);


        // Command dispatch
        switch(xmainthread.cmd)
        {
          
        case Communicator::Command::RenderThreadAddFrame:
        {
        }
        break;

        case Communicator::Command::FrameLayout:
        {
          auto &frame=*xmainthread.framemap[xmainthread.iframe];
          xmainthread.communicator->SendInt(frame.parameter.nvpx);
          xmainthread.communicator->SendInt(frame.parameter.nvpy);
          frame.nvpx=frame.parameter.nvpx;
          frame.nvpy=frame.parameter.nvpy;
        }
        break;

        case Communicator::Command::FrameActiveSubFrame:
        {
          auto &frame=*xmainthread.framemap[xmainthread.iframe];
          xmainthread.communicator->SendInt(frame.parameter.active_subframe);
        }
        break;

        case Communicator::Command::RenderThreadRemoveFrame:
        {

        }
        break;

        case Communicator::Command::RenderThreadShow:
        {
          for (auto framepair: xmainthread.framemap)
            for (auto & figure: framepair.second->figures)
            {
              figure->SetRange();
              figure->ServerMPSendData(xmainthread.communicator);
            }
        }
        break;
        
        case Communicator::Command::FrameDump:
        {
          
          auto &frame=*xmainthread.framemap[xmainthread.iframe];
          xmainthread.communicator->SendString(frame.parameter.filename);
        }
        break;
        
        case Communicator::Command::FrameSize:
        {
          auto &frame=*xmainthread.framemap[xmainthread.iframe];
          xmainthread.communicator->SendInt(frame.parameter.winsize_x);
          xmainthread.communicator->SendInt(frame.parameter.winsize_y);
        }
        break;

        case Communicator::Command::FramePosition:
        {
          auto &frame=*xmainthread.framemap[xmainthread.iframe];
          xmainthread.communicator->SendInt(frame.parameter.winposition_x);
          xmainthread.communicator->SendInt(frame.parameter.winposition_y);
        }
        break;

        case Communicator::Command::FrameTitle:
        {
          auto &frame=*xmainthread.framemap[xmainthread.iframe];
          xmainthread.communicator->SendString(frame.parameter.frametitle);
        }
        break;


        case Communicator::Command::WindowTitle:
        {
          auto &frame=*xmainthread.framemap[xmainthread.iframe];
          xmainthread.communicator->SendString(frame.parameter.wintitle);
        }
        break;


        case Communicator::Command::FrameAddFigure:
        {
          auto &frame=*xmainthread.framemap[xmainthread.iframe];
          xmainthread.communicator->SendString(frame.parameter.current_figure->SubClassName());
          xmainthread.communicator->SendInt(frame.parameter.current_figure->framepos);
        }
        break;

        case Communicator::Command::FrameRemoveFigure:
        {
          auto &frame=*xmainthread.framemap[xmainthread.iframe];
          xmainthread.communicator->SendString(frame.parameter.current_figure->SubClassName());
          xmainthread.communicator->SendInt(frame.parameter.current_figure->framepos);
        }
        break;


        case Communicator::Command::FrameLinkCamera:
        {
          auto &frame=*xmainthread.framemap[xmainthread.iframe];
          xmainthread.communicator->SendInt(frame.parameter.camlinkthisframepos);
          xmainthread.communicator->SendInt(frame.parameter.camlinkframenum);
          xmainthread.communicator->SendInt(frame.parameter.camlinkframepos);
        }
        break;

        case Communicator::Command::Clear:
        {
          // for (auto & figure: frame.figures)
          // {
            
          //   //frame.communicator->SendCommand(::Command::FrameShow);
          // }
          
        }
        break;
        
        case Communicator::Command::RenderThreadTerminate:
          // Close window and terminate
        {

          if (xmainthread.debug_level>0)
            cout << ": server termination" << endl;
          xmainthread.framemap.clear();
          // Notify that command was exeuted
          xmainthread.condition_variable.notify_all();
          xmainthread.running_multithreaded=false;
          return;
        }
        break;
        
        default:
        {
          cout << ": server cmd: " << static_cast<int>(xmainthread.cmd) << endl;
          throw std::runtime_error("wrong command on server");
        }
        
        break;
        }
        
        // Clear command
        xmainthread.cmd=Communicator::Command::Empty;
        
        // Notify that command was exeuted
        if (xmainthread.running_multithreaded)
          xmainthread.condition_variable.notify_all();
      }

    }

  

    
/////////////////////////////////////////////////////////////////////////////////////////////////
    
    ///
    /// Main communication thread.
    ///
    /// Not to be exposed to user.
    class  XRenderThread
    {
      friend class Frame;
      friend class Client;
      friend class TimerCallback;
      friend class InteractorStyle;

    public:
      ~XRenderThread();
      XRenderThread();
      
      /// Find frame with number
      static Frame& FindFrame(int number_in_framelist);


    private:
       vtkSmartPointer<vtkRenderWindowInteractor> interactor;


      /// Start thread.
      ///XC
      /// Create a singleton object. We need only
      /// as vtk is not thread save.
      static std::shared_ptr<XRenderThread> CreateXRenderThread();

      /// "This" thread
      static std::shared_ptr<XRenderThread> xmainthread;
      
      /// Kill the thread
      static void DeleteXRenderThread();

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
      static void PrepareXRenderThread(XRenderThread& thd);

      /// Callback function for render thread
      static void XRenderThreadCallback(XRenderThread& thd);

      /// Prepare comm thread before start
      static void PrepareCommunicatorThread(XRenderThread& thd);

      /// Callback function for communicator thread
      static void CommunicatorThread(XRenderThread& thd);

      /// Callback function for communicator thread 
      /// if multithreading is off
      static void CommunicatorThreadCallback(XRenderThread& thd);

      
      /// Add frame, to be called from render thread
      static void RTAddFrame(XRenderThread& mt, int iframe);

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



    std::shared_ptr<XRenderThread> XRenderThread::xmainthread;


    void XRenderThread::DeleteXRenderThread()
    {
      
      if (xmainthread==nullptr)        return;
      xmainthread->Terminate();
      if (xmainthread->debug_level>0)
        std::cout << ": terminated" << std::endl;
      xmainthread=nullptr;
    }

    
    
    std::shared_ptr<XRenderThread> XRenderThread::CreateXRenderThread()
    {
      if (xmainthread==0)
        xmainthread=std::make_shared<XRenderThread>();
      std::atexit(DeleteXRenderThread);
      return xmainthread;
    }

    XRenderThread::XRenderThread()
    {
      int wtime=5;
      char* port_string= getenv("_PORT_NUMBER");
      char* wtime_string=getenv("_WAIT_SECONDS");
      char *debug_string=getenv("_DEBUG");
      char *multi_string=getenv("_MULTITHREADED");
      char *dbuff_string=getenv("_DOUBLEBUFFER");



      if (debug_string!=0)
        debug_level=atoi(debug_string);

    
#if XXXCONFIG_APPLE
      try_running_multithreaded=false;
      if (multi_string!=0 && atoi(multi_string))
      {
        if (debug_level>0)
          cout << ": overriding multithreading default (off) on APPLE" << endl;
        try_running_multithreaded=true;
      }
#endif
      try_running_multithreaded=true;
      if (multi_string!=0 && !atoi(multi_string))
      {
        if (debug_level>0)
          cout << ": overriding multithreading default (on)" << endl;
        try_running_multithreaded=false;
      }
      
      if (dbuff_string!=0)
        this->double_buffering=atoi(dbuff_string);
      

      if (wtime_string!=0)
        wtime=atoi(wtime_string);
    
    
      if (port_string!=0)
      {
        int port=atoi(port_string);
        if (port<=0) 
          throw std::runtime_error(": Invalid port number for server");
        OpenConnection(port,wtime);
      }

      if (debug_level>0)
      {   
        std::string sc="client";
        if (connection_open) 
          sc="server";

        if (try_running_multithreaded)
          cout << ": try running multithreaded as " << sc << endl;
        else
          cout << ": try running single threaded as " << sc << endl;
      }
    }


    XRenderThread::~XRenderThread()
    {
      if (debug_level>0)
        cout << ": ~XRenderThread"  << endl;

      // if (this->running_multithreaded)
      // {
      //   std::this_thread::sleep_for(std::chrono::milliseconds(10));
      //   Terminate();
      //   std::this_thread::sleep_for(std::chrono::milliseconds(10));
      if (this->thread)
        this->thread->join();
      // }
      // else
      //   Terminate();

      // framemap.clear();
    }


   

    void  XRenderThread::Update()
    {
      if (connection_open)
        CommunicatorThreadCallback(*this);
      else if (interactor)
        interactor->Start();
    }
  
    void  XRenderThread::AddFrame(std::shared_ptr<Frame> frame)
    {
      frame->number_in_frame_list=lastframenum;
      this->iframe=lastframenum;
      framemap[lastframenum++]=frame;
      if (lastframenum==1)
      {
        if (connection_open)
        {
          PrepareCommunicatorThread(*this);
          CommunicatorThreadCallback(*this);
        }
        else if (try_running_multithreaded)
        {
          Start();
        }
        else
        {
          PrepareXRenderThread(*this);
        }
      }
      else
      {
        SendCommand(frame->number_in_frame_list, "AddFrame", Communicator::Command::RenderThreadAddFrame);
      }
    }


    void XRenderThread::RemoveFrame(int number_in_framelist)
    {
      SendCommand(number_in_framelist, "RemoveFrame", Communicator::Command::RenderThreadRemoveFrame);
      std::this_thread::sleep_for (std::chrono::milliseconds(10));
    }

    Frame& XRenderThread::FindFrame(int number_in_framelist)
    {
      auto &frame=*xmainthread->framemap[number_in_framelist];
      return frame;
    }
    
    void XRenderThread::Show()
    {
      SendCommand(-1, "Show", Communicator::Command::RenderThreadShow);
    }


    void XRenderThread::Interact()
    {
      
      SendCommand(-1, "ShowAndBlock", Communicator::Command::RenderThreadShowAndBlock);

      do
      {
        std::this_thread::sleep_for (std::chrono::milliseconds(10));
        if (!this->running_multithreaded)
          Show();
      }
      while (this->communication_blocked);

    }


    void XRenderThread::SendCommand(int number_in_frame_list, const std::string from, Communicator::Command cmd)
    {
      if (debug_level>0)
        cout << ": XRenderThread::SendCommand " << from << " " << number_in_frame_list << endl;


      this->cmd=cmd;
      this->iframe=number_in_frame_list;
      if (running_multithreaded)
      {
        std::unique_lock<std::mutex> lock(this->mutex);
        this->condition_variable.wait(lock);
      }
      else
        Update();
    }

    void XRenderThread::Terminate(void)
    {
      SendCommand(-1,"Terminate",Communicator::Command::RenderThreadTerminate);
    }
  

    void XRenderThread::RTAddFrame(XRenderThread& xmainthread, int iframe)
    {
      auto &frame=*xmainthread.framemap[iframe];
      frame.window = vtkSmartPointer<vtkRenderWindow>::New();
      frame.window->SetWindowName("");

      if (xmainthread.double_buffering)
        frame.window->DoubleBufferOn();
      else
        frame.window->DoubleBufferOff();

      frame.window->SetSize(frame.parameter.winsize_x, frame.parameter.winsize_y);
      frame.window->SetPosition(frame.parameter.winposition_x, frame.parameter.winposition_y);
      
      frame.RTResetRenderers(true);

      frame.RTInit();
      frame.title_subframe.renderer->AddActor(frame.title_actor);
      frame.title_actor->SetText(6,"---");
      frame.title_actor->SetText(7,frame.parameter.frametitle.c_str());

    }

    void XRenderThread::PrepareXRenderThread(XRenderThread& xmainthread)
    {
      RTAddFrame(xmainthread,0);
      auto &frame=*xmainthread.framemap[0];

      xmainthread.interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
      auto style =  vtkSmartPointer<InteractorStyle>::New();
      style->SetFrame(&frame);
      xmainthread.interactor->SetInteractorStyle(style);
    
      xmainthread.interactor->SetRenderWindow(frame.window);
      auto callback =  vtkSmartPointer<TimerCallback>::New();
      //!!! callback->mainthread=&xmainthread;
      callback->Interactor=xmainthread.interactor;
      xmainthread.interactor->AddObserver(vtkCommand::TimerEvent,callback);

      xmainthread.interactor->Initialize();

      xmainthread.interactor->CreateRepeatingTimer(xmainthread.timer_interval);
    }
  
    void XRenderThread::XRenderThreadCallback(XRenderThread& xmainthread)
    {

      if (xmainthread.debug_level>0)
        cout << ": XRenderThread start" << endl;

      XRenderThread::PrepareXRenderThread(xmainthread);
      xmainthread.running_multithreaded=true;

      xmainthread.interactor->Start();
      if (xmainthread.debug_level>0)
        cout << ": XRenderThread prepare termination" << endl;

      for (auto&  frame :xmainthread.framemap)
      {
        for (auto & subframe: frame.second->subframes)
          subframe.renderer->RemoveAllViewProps();
        frame.second->window->Finalize();
      }

      xmainthread.running_multithreaded=false;
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      xmainthread.condition_variable.notify_all();

      for (auto &frame: xmainthread.framemap)
        frame.second->window=0;
      
      xmainthread.interactor=0;
      
      if (xmainthread.debug_level>0)
        cout << ": XRenderThread stop" << endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    void XRenderThread::Start(void)
    {
      if (connection_open)
        this->thread=std::make_shared<std::thread>(CommunicatorThread,std::ref(*this));
      else
        this->thread=std::make_shared<std::thread>(XRenderThreadCallback,std::ref(*this));

      do
      {
        std::this_thread::sleep_for (std::chrono::milliseconds(10));
      }
      while (!this->running_multithreaded);
    }


    ////////////////////////////////////////////////////////////////
    /// Server communication 

    void XRenderThread::OpenConnection(int port, int wtime)
    {
      if (connection_open) return;

      cout << ": Server start listening on port "<< port << endl;
      communicator=vtkSmartPointer<Communicator>::New();
      communicator->server_listen_num_retry=1;
      communicator->server_listen_waiting_time=1000*wtime;
      int rc=communicator->ServerMPConnect(port);
      if (rc)  
        cout << ": Server connected" << endl;
      else
      {
        cout << ": Nobody is listening ... giving up" << endl;
        throw std::runtime_error("Server connection failed");
        exit(1);
      }
      connection_open=true;
    }

    void  XRenderThread::PrepareCommunicatorThread(XRenderThread& xmainthread)
    {
      xmainthread.cmd=Communicator::Command::RenderThreadAddFrame;

    }

    void  XRenderThread::CommunicatorThreadCallback(XRenderThread& xmainthread)
    {
      if (xmainthread.cmd!=Communicator::Command::Empty)
      {      
        
        // Lock mutex
        if (xmainthread.running_multithreaded)
          std::unique_lock<std::mutex> lock(xmainthread.mutex);

//        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        if (xmainthread.debug_level>0) 
          cout << ": send cmd: " << static_cast<int>(xmainthread.cmd) << " frame: " <<xmainthread.iframe<< endl;


        xmainthread.communicator->SendCommand(xmainthread.cmd);
        xmainthread.communicator->SendInt(xmainthread.iframe);


        // Command dispatch
        switch(xmainthread.cmd)
        {
          
        case Communicator::Command::RenderThreadAddFrame:
        {
        }
        break;

        case Communicator::Command::FrameLayout:
        {
          auto &frame=*xmainthread.framemap[xmainthread.iframe];
          xmainthread.communicator->SendInt(frame.parameter.nvpx);
          xmainthread.communicator->SendInt(frame.parameter.nvpy);
          frame.nvpx=frame.parameter.nvpx;
          frame.nvpy=frame.parameter.nvpy;
        }
        break;

        case Communicator::Command::FrameActiveSubFrame:
        {
          auto &frame=*xmainthread.framemap[xmainthread.iframe];
          xmainthread.communicator->SendInt(frame.parameter.active_subframe);
        }
        break;

        case Communicator::Command::RenderThreadRemoveFrame:
        {

        }
        break;

        case Communicator::Command::RenderThreadShow:
        {
          for (auto framepair: xmainthread.framemap)
            for (auto & figure: framepair.second->figures)
            {
              figure->SetRange();
              figure->ServerMPSendData(xmainthread.communicator);
            }
        }
        break;
        
        case Communicator::Command::FrameDump:
        {
          
          auto &frame=*xmainthread.framemap[xmainthread.iframe];
          xmainthread.communicator->SendString(frame.parameter.filename);
        }
        break;
        
        case Communicator::Command::FrameSize:
        {
          auto &frame=*xmainthread.framemap[xmainthread.iframe];
          xmainthread.communicator->SendInt(frame.parameter.winsize_x);
          xmainthread.communicator->SendInt(frame.parameter.winsize_y);
        }
        break;

        case Communicator::Command::FramePosition:
        {
          auto &frame=*xmainthread.framemap[xmainthread.iframe];
          xmainthread.communicator->SendInt(frame.parameter.winposition_x);
          xmainthread.communicator->SendInt(frame.parameter.winposition_y);
        }
        break;

        case Communicator::Command::FrameTitle:
        {
          auto &frame=*xmainthread.framemap[xmainthread.iframe];
          xmainthread.communicator->SendString(frame.parameter.frametitle);
        }
        break;


        case Communicator::Command::WindowTitle:
        {
          auto &frame=*xmainthread.framemap[xmainthread.iframe];
          xmainthread.communicator->SendString(frame.parameter.wintitle);
        }
        break;


        case Communicator::Command::FrameAddFigure:
        {
          auto &frame=*xmainthread.framemap[xmainthread.iframe];
          xmainthread.communicator->SendString(frame.parameter.current_figure->SubClassName());
          xmainthread.communicator->SendInt(frame.parameter.current_figure->framepos);
        }
        break;

        case Communicator::Command::FrameRemoveFigure:
        {
          auto &frame=*xmainthread.framemap[xmainthread.iframe];
          xmainthread.communicator->SendString(frame.parameter.current_figure->SubClassName());
          xmainthread.communicator->SendInt(frame.parameter.current_figure->framepos);
        }
        break;


        case Communicator::Command::FrameLinkCamera:
        {
          auto &frame=*xmainthread.framemap[xmainthread.iframe];
          xmainthread.communicator->SendInt(frame.parameter.camlinkthisframepos);
          xmainthread.communicator->SendInt(frame.parameter.camlinkframenum);
          xmainthread.communicator->SendInt(frame.parameter.camlinkframepos);
        }
        break;

        case Communicator::Command::Clear:
        {
          // for (auto & figure: frame.figures)
          // {
            
          //   //frame.communicator->SendCommand(::Command::FrameShow);
          // }
          
        }
        break;
        
        case Communicator::Command::RenderThreadTerminate:
          // Close window and terminate
        {

          if (xmainthread.debug_level>0)
            cout << ": server termination" << endl;
          xmainthread.framemap.clear();
          // Notify that command was exeuted
          xmainthread.condition_variable.notify_all();
          xmainthread.running_multithreaded=false;
          return;
        }
        break;
        
        default:
        {
          cout << ": server cmd: " << static_cast<int>(xmainthread.cmd) << endl;
          throw std::runtime_error("wrong command on server");
        }
        
        break;
        }
        
        // Clear command
        xmainthread.cmd=Communicator::Command::Empty;
        
        // Notify that command was exeuted
        if (xmainthread.running_multithreaded)
          xmainthread.condition_variable.notify_all();
      }

    }

    void  XRenderThread::CommunicatorThread(XRenderThread& xmainthread)
    {
      xmainthread.running_multithreaded=true;
      PrepareCommunicatorThread(xmainthread);
      while(1)
      {
        std::this_thread::sleep_for (std::chrono::milliseconds(5));
        XRenderThread::CommunicatorThreadCallback(xmainthread);
      }
      xmainthread.running_multithreaded=false;
    }
  
 
  
  }

}
