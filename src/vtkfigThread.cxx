#include <stdexcept>
#ifdef __APPLE__
#include <unistd.h>
#endif



#include <vtkCommand.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkObjectBase.h>
#include <vtkPropCollection.h>
#include "config.h"

#ifdef QT
#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>
#endif



#include "vtkfigFrame.h"
#include "vtkfigFigure.h"

#include "internals/vtkfigThread.h"
#include "internals/vtkfigInteractorStyle.h"
#include "internals/vtkfigTimerCallback.h"

namespace vtkfig
{
  namespace internals
  {
    std::shared_ptr<Thread> Thread::mainthread;


    void Thread::KillThread()
    {
      
      if (mainthread==nullptr)        return;
      mainthread->Terminate();
      if (mainthread->debug_level>0)
        std::cout << "vtkfig: terminated" << std::endl;
      mainthread=nullptr;
    }

    
    
    std::shared_ptr<Thread> Thread::CreateThread()
    {
      if (mainthread==0)
        mainthread=std::make_shared<Thread>();
      std::atexit(KillThread);
      return mainthread;
    }

    Thread::Thread()
    {
      int wtime=5;
      char* port_string= getenv("VTKFIG_PORT_NUMBER");
      char* wtime_string=getenv("VTKFIG_WAIT_SECONDS");
      char *debug_string=getenv("VTKFIG_DEBUG");
      char *multi_string=getenv("VTKFIG_MULTITHREADED");
      char *dbuff_string=getenv("VTKFIG_DOUBLEBUFFER");



      if (debug_string!=0)
        debug_level=atoi(debug_string);

    
#if XXXCONFIG_APPLE
      try_running_multithreaded=false;
      if (multi_string!=0 && atoi(multi_string))
      {
        if (debug_level>0)
          cout << "vtkfig: overriding multithreading default (off) on APPLE" << endl;
        try_running_multithreaded=true;
      }
#endif
      try_running_multithreaded=true;
      if (multi_string!=0 && !atoi(multi_string))
      {
        if (debug_level>0)
          cout << "vtkfig: overriding multithreading default (on)" << endl;
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
          throw std::runtime_error("vtkfig: Invalid port number for server");
        OpenConnection(port,wtime);
      }

      if (debug_level>0)
      {   
        std::string sc="client";
        if (connection_open) 
          sc="server";

        if (try_running_multithreaded)
          cout << "vtkfig: try running multithreaded as " << sc << endl;
        else
          cout << "vtkfig: try running single threaded as " << sc << endl;
      }
    }


    Thread::~Thread()
    {
      if (debug_level>0)
        cout << "vtkfig: ~RenderThread"  << endl;

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


   

    void  Thread::Update()
    {
      if (connection_open)
        CommunicatorThreadCallback(*this);
      else if (interactor)
        interactor->Start();
    }
  
    void  Thread::AddFrame(std::shared_ptr<Frame> frame)
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
          PrepareRenderThread(*this);
        }
      }
      else
      {
        SendCommand(frame->number_in_frame_list, "AddFrame", Communicator::Command::RenderThreadAddFrame);
      }
    }


    void Thread::RemoveFrame(int number_in_framelist)
    {
      SendCommand(number_in_framelist, "RemoveFrame", Communicator::Command::RenderThreadRemoveFrame);
      std::this_thread::sleep_for (std::chrono::milliseconds(10));
    }

    Frame& Thread::FindFrame(int number_in_framelist)
    {
      auto &frame=*mainthread->framemap[number_in_framelist];
      return frame;
    }
    
    void Thread::Show()
    {
      SendCommand(-1, "Show", Communicator::Command::RenderThreadShow);
    }


    void Thread::Interact()
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


    void Thread::SendCommand(int number_in_frame_list, const std::string from, Communicator::Command cmd)
    {
      if (debug_level>0)
        cout << "vtkfig: Thread::SendCommand " << from << " " << number_in_frame_list << endl;


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

    void Thread::Terminate(void)
    {
      SendCommand(-1,"Terminate",Communicator::Command::RenderThreadTerminate);
    }
  

    void Thread::RTAddFrame(Thread& mainthread, int iframe)
    {
      auto &frame=*mainthread.framemap[iframe];
      frame.window = vtkSmartPointer<Window>::New();
      frame.window->SetWindowName("vtkfig");

      if (mainthread.double_buffering)
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

    void Thread::PrepareRenderThread(Thread& mainthread)
    {
      RTAddFrame(mainthread,0);
      auto &frame=*mainthread.framemap[0];

      mainthread.interactor = vtkSmartPointer<Interactor>::New();
      auto style =  vtkSmartPointer<InteractorStyle>::New();
      style->SetFrame(&frame);
      mainthread.interactor->SetInteractorStyle(style);
    
      mainthread.interactor->SetRenderWindow(frame.window);
      auto callback =  vtkSmartPointer<TimerCallback>::New();
      callback->mainthread=&mainthread;
      callback->Interactor=mainthread.interactor;
      mainthread.interactor->AddObserver(vtkCommand::TimerEvent,callback);

      mainthread.interactor->CreateRepeatingTimer(mainthread.timer_interval);
    }
  
    void Thread::RenderThreadCallback(Thread& mainthread)
    {

      if (mainthread.debug_level>0)
        cout << "vtkfig: RenderThread start" << endl;

#ifdef QT
      // QApplication needs to be constructed before anything QT
      // is performed
      int argc=1;
      char arg0[]={'x','\0'};
      char *argv[]={arg0};
      QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
      QApplication app(argc,argv);
#endif
      Thread::PrepareRenderThread(mainthread);
      mainthread.running_multithreaded=true;

#ifdef QT
      auto &frame=*mainthread.framemap[0];
      QVTKOpenGLNativeWidget widget;
      mainthread.interactor->Initialize();
      frame.window->SetInteractor(mainthread.interactor);
      widget.setRenderWindow(frame.window);
      widget.show();
      app.exec();
#else      
      mainthread.interactor->Initialize();
      mainthread.interactor->Start();
#endif


      if (mainthread.debug_level>0)
        cout << "vtkfig: RenderThread prepare termination" << endl;

      for (auto&  frame :mainthread.framemap)
      {
        for (auto & subframe: frame.second->subframes)
          subframe.renderer->RemoveAllViewProps();
        frame.second->window->Finalize();
      }

      mainthread.running_multithreaded=false;
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      mainthread.condition_variable.notify_all();

      for (auto &frame: mainthread.framemap)
        frame.second->window=0;
      
      mainthread.interactor=0;
      
      if (mainthread.debug_level>0)
        cout << "vtkfig: RenderThread stop" << endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    void Thread::Start(void)
    {

      if (connection_open)
        this->thread=std::make_shared<std::thread>(CommunicatorThread,std::ref(*this));
      else
        this->thread=std::make_shared<std::thread>(RenderThreadCallback,std::ref(*this));
      
      do
      {
        std::this_thread::sleep_for (std::chrono::milliseconds(10));
      }
      while (!this->running_multithreaded);
    }


    ////////////////////////////////////////////////////////////////
    /// Server communication 

    void Thread::OpenConnection(int port, int wtime)
    {
      if (connection_open) return;

      cout << "vtkfig: Server start listening on port "<< port << endl;
      communicator=vtkSmartPointer<Communicator>::New();
      communicator->server_listen_num_retry=1;
      communicator->server_listen_waiting_time=1000*wtime;
      int rc=communicator->ServerMPConnect(port);
      if (rc)  
        cout << "vtkfig: Server connected" << endl;
      else
      {
        cout << "vtkfig: Nobody is listening ... giving up" << endl;
        throw std::runtime_error("Server connection failed");
        exit(1);
      }
      connection_open=true;
    }

    void  Thread::PrepareCommunicatorThread(Thread& mainthread)
    {
      mainthread.cmd=Communicator::Command::RenderThreadAddFrame;

    }

    void  Thread::CommunicatorThreadCallback(Thread& mainthread)
    {
      if (mainthread.cmd!=Communicator::Command::Empty)
      {      
        
        // Lock mutex
        if (mainthread.running_multithreaded)
          std::unique_lock<std::mutex> lock(mainthread.mutex);

//        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        if (mainthread.debug_level>0) 
          cout << "vtkfig: send cmd: " << static_cast<int>(mainthread.cmd) << " frame: " <<mainthread.iframe<< endl;


        mainthread.communicator->SendCommand(mainthread.cmd);
        mainthread.communicator->SendInt(mainthread.iframe);


        // Command dispatch
        switch(mainthread.cmd)
        {
          
        case Communicator::Command::RenderThreadAddFrame:
        {
        }
        break;

        case Communicator::Command::FrameLayout:
        {
          auto &frame=*mainthread.framemap[mainthread.iframe];
          mainthread.communicator->SendInt(frame.parameter.nvpx);
          mainthread.communicator->SendInt(frame.parameter.nvpy);
          frame.nvpx=frame.parameter.nvpx;
          frame.nvpy=frame.parameter.nvpy;
        }
        break;

        case Communicator::Command::FrameActiveSubFrame:
        {
          auto &frame=*mainthread.framemap[mainthread.iframe];
          mainthread.communicator->SendInt(frame.parameter.active_subframe);
        }
        break;

        case Communicator::Command::RenderThreadRemoveFrame:
        {

        }
        break;

        case Communicator::Command::RenderThreadShow:
        {
          for (auto framepair: mainthread.framemap)
            for (auto & figure: framepair.second->figures)
            {
              figure->SetRange();
              figure->ServerMPSendData(mainthread.communicator);
            }
        }
        break;
        
        case Communicator::Command::FrameDump:
        {
          
          auto &frame=*mainthread.framemap[mainthread.iframe];
          mainthread.communicator->SendString(frame.parameter.filename);
        }
        break;
        
        case Communicator::Command::FrameSize:
        {
          auto &frame=*mainthread.framemap[mainthread.iframe];
          mainthread.communicator->SendInt(frame.parameter.winsize_x);
          mainthread.communicator->SendInt(frame.parameter.winsize_y);
        }
        break;

        case Communicator::Command::FramePosition:
        {
          auto &frame=*mainthread.framemap[mainthread.iframe];
          mainthread.communicator->SendInt(frame.parameter.winposition_x);
          mainthread.communicator->SendInt(frame.parameter.winposition_y);
        }
        break;

        case Communicator::Command::FrameTitle:
        {
          auto &frame=*mainthread.framemap[mainthread.iframe];
          mainthread.communicator->SendString(frame.parameter.frametitle);
        }
        break;


        case Communicator::Command::WindowTitle:
        {
          auto &frame=*mainthread.framemap[mainthread.iframe];
          mainthread.communicator->SendString(frame.parameter.wintitle);
        }
        break;


        case Communicator::Command::FrameAddFigure:
        {
          auto &frame=*mainthread.framemap[mainthread.iframe];
          mainthread.communicator->SendString(frame.parameter.current_figure->SubClassName());
          mainthread.communicator->SendInt(frame.parameter.current_figure->framepos);
        }
        break;

        case Communicator::Command::FrameRemoveFigure:
        {
          auto &frame=*mainthread.framemap[mainthread.iframe];
          mainthread.communicator->SendString(frame.parameter.current_figure->SubClassName());
          mainthread.communicator->SendInt(frame.parameter.current_figure->framepos);
        }
        break;


        case Communicator::Command::FrameLinkCamera:
        {
          auto &frame=*mainthread.framemap[mainthread.iframe];
          mainthread.communicator->SendInt(frame.parameter.camlinkthisframepos);
          mainthread.communicator->SendInt(frame.parameter.camlinkframenum);
          mainthread.communicator->SendInt(frame.parameter.camlinkframepos);
        }
        break;

        case Communicator::Command::Clear:
        {
          // for (auto & figure: frame.figures)
          // {
            
          //   //frame.communicator->SendCommand(vtkfig::Command::FrameShow);
          // }
          
        }
        break;
        
        case Communicator::Command::RenderThreadTerminate:
          // Close window and terminate
        {

          if (mainthread.debug_level>0)
            cout << "vtkfig: server termination" << endl;
          mainthread.framemap.clear();
          // Notify that command was exeuted
          mainthread.condition_variable.notify_all();
          mainthread.running_multithreaded=false;
          return;
        }
        break;
        
        default:
        {
          cout << "vtkfig: server cmd: " << static_cast<int>(mainthread.cmd) << endl;
          throw std::runtime_error("wrong command on server");
        }
        
        break;
        }
        
        // Clear command
        mainthread.cmd=Communicator::Command::Empty;
        
        // Notify that command was exeuted
        if (mainthread.running_multithreaded)
          mainthread.condition_variable.notify_all();
      }

    }

    void  Thread::CommunicatorThread(Thread& mainthread)
    {
      mainthread.running_multithreaded=true;
      PrepareCommunicatorThread(mainthread);
      while(1)
      {
        std::this_thread::sleep_for (std::chrono::milliseconds(5));
        Thread::CommunicatorThreadCallback(mainthread);
      }
      mainthread.running_multithreaded=false;
    }
  
 
  
  }

}
