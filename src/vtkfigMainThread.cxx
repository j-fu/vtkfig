#include <stdexcept>
#include <chrono>

#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkWindowToImageFilter.h"
#include "vtkPNGWriter.h"
#include "vtkCommand.h"
#include "vtkProperty.h"
#include "vtkProperty2D.h"
#include "vtkObjectBase.h"
#include "vtkPropCollection.h"




#include "vtkfigFrame.h"
#include "vtkfigFigure.h"
#include "vtkfigMainThread.h"
#include "config.h"


namespace vtkfig
{
  namespace internals
  {
    MainThread * MainThread::mainthread=0;

    void MainThread::DeleteMainThread()
    {

      if (mainthread==0)
        return;
      delete mainthread;
      mainthread=0;
    }


    MainThread* MainThread::CreateMainThread()
    {
      if (mainthread==0)
        mainthread=new MainThread();
      std::atexit(DeleteMainThread);
      return mainthread;
    }

    MainThread::MainThread()
    {
      int wtime=5;
      char* port_string= getenv("VTKFIG_PORT_NUMBER");
      char* wtime_string=getenv("VTKFIG_WAIT_SECONDS");
      char *debug_string=getenv("VTKFIG_DEBUG");
      char *multi_string=getenv("VTKFIG_MULTITHREADED");
      char *dbuff_string=getenv("VTKFIG_DOUBLEBUFFER");



      if (debug_string!=0)
        debug_level=atoi(debug_string);

    
#if CONFIG_APPLE
      try_running_multithreaded=false;
      if (multi_string!=0 && atoi(multi_string))
      {
        if (debug_level>0)
          cout << "overriding multithreading default (off) on APPLE" << endl;
        try_running_multithreaded=true;
      }
#else
      try_running_multithreaded=true;
      if (multi_string!=0 && !atoi(multi_string))
      {
        if (debug_level>0)
          cout << "overriding multithreading default (on) on NON-APPLE" << endl;
        try_running_multithreaded=false;
      }
#endif

      if (dbuff_string!=0)
        this->double_buffering=atoi(dbuff_string);
      

      if (wtime_string!=0)
        wtime=atoi(wtime_string);
    
    
      if (port_string!=0)
      {
        int port=atoi(port_string);
        if (port<=0) 
          throw std::runtime_error("Invalid port number for server");
        OpenConnection(port,wtime);
      }

      if (debug_level>0)
      {   
        char sc='c';
        if (connection_open) 
          sc='s';

        if (try_running_multithreaded)
          cout << sc<< " try running multithreaded" << endl;
        else
          cout << sc<<" try running single threaded" << endl;
      }
    }


    void MainThread::OpenConnection(int port, int wtime)
    {
      if (connection_open) return;

      cout << "Server start listening on port "<< port << endl;
      communicator=vtkSmartPointer<Communicator>::New();
      communicator->server_listen_num_retry=1;
      communicator->server_listen_waiting_time=1000*wtime;
      int rc=communicator->ServerConnect(port);
      if (rc)  
        cout << "Server connected" << endl;
      else
      {
        cout << "Nobody is listening ... giving up" << endl;
        throw std::runtime_error("Server connection failed");
        exit(1);
      }
      connection_open=true;
    }
   
    MainThread::~MainThread()
    {
      if (debug_level>0)
        cout << " ~mt"  << endl;
      if (this->running_multithreaded)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        Terminate();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        this->thread->join();
      }
      else
        Terminate();

      framemap.clear();
    }



    void  MainThread::Update()
    {
      if (connection_open)
        CommunicatorThreadCallback(this);
      else
        interactor->Start();
    }
  
    void  MainThread::AddFrame(Frame*frame)
    {
      frame->mainthread=this;
      frame->number_in_frame_list=lastframenum;
      this->iframe=lastframenum;
      framemap[lastframenum++]=frame;
      if (lastframenum==1)
      {
        if (try_running_multithreaded)
        {
          Start();
        }
        else
        {
          if (connection_open)
          {
            PrepareCommunicatorThread(this);
            CommunicatorThreadCallback(this);
          }
          else
          {
            PrepareRenderThread(this);
          }
        }
      }
      else
      {
        SendCommand(frame->number_in_frame_list, "AddFrame", Communicator::Command::MainThreadAddFrame);
      }
    }


    void MainThread::RemoveFrame(Frame *frame)
    {
      assert(frame==framemap[frame->number_in_frame_list]);
      SendCommand(frame->number_in_frame_list, "RemoveFrame", Communicator::Command::MainThreadRemoveFrame);
      frame->number_in_frame_list=-1;
    }

    void MainThread::Show()
    {
      SendCommand(-1, "Show", Communicator::Command::MainThreadShow);
    }


    void MainThread::Interact()
    {
      // if (!this->running_multithreaded)
      //   throw std::runtime_error("Interact: render thread is dead");

      SendCommand(-1, "Show", Communicator::Command::MainThreadShow);

      this->communication_blocked=true;
      do
      {
        std::this_thread::sleep_for (std::chrono::milliseconds(10));
        if (!this->running_multithreaded)
          Show();
      }
      while (this->communication_blocked);

    }


    void MainThread::SendCommand(int number_in_frame_list, const std::string from, Communicator::Command cmd)
    {
      if (debug_level>0)
        cout << "mt " << from << " " << number_in_frame_list << endl;


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

    void MainThread::Terminate(void)
    {
      SendCommand(-1,"Terminate",Communicator::Command::MainThreadTerminate);
    }

  
    
    ///
    ///  vtkfig specific keybord and mouse interaction
    ///
    class MyInteractorStyle : public vtkInteractorStyleTrackballCamera
    {

      /// List of currently edited figures
      std::vector<Figure*>edited_figures;
      
      /// Figure edit mode
      bool edit_mode =false;

      /// Mouse button down 
      bool left_button_down=false;

      /// Last mouse  x position
      int lastx=0;

      /// Last mouse  y position
      int lasty=0;
      

      /// Current frame
      Frame *frame=0;

      
    public:

      
    
      static MyInteractorStyle* New()
      {
        return new MyInteractorStyle();
      }

      MyInteractorStyle(): vtkInteractorStyleTrackballCamera() {};

      void SetFrame(Frame *f)
      {
        this->frame=f;
      }
      
      /// Overwrite left button down
      virtual void OnLeftButtonDown()
      {

        /// If edit mode is active,
        /// catch mouse position, otherwisw pass to base class
        if (this->edit_mode)
        {
          this->left_button_down=true;
          lastx=this->Interactor->GetEventPosition()[0];
          lasty=this->Interactor->GetEventPosition()[1];
        }
        else
          vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
      }

      /// Overwrite left button up
      virtual void OnLeftButtonUp()
      {
        /// If button was down, release, else pass to base class
        if (this->left_button_down)
          this->left_button_down=false;
        else
          vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
      }

      /// Overwrite  mouse move
      virtual void OnMouseMove()
      {

        
        /// If button is down, act, else pass to base class
        if (this->left_button_down)
        {
          // Calculate difference between old and
          // new mouse position
          int thisx=this->Interactor->GetEventPosition()[0];
          int thisy=this->Interactor->GetEventPosition()[1];
          int dx=thisx-lastx;
          int dy=thisy-lasty;

          // Pass to all edited figures
          for (auto figure:this->edited_figures) 
            figure->RTProcessMove(dx,dy);
          
          // Render changed figure
          this->Interactor->Render();

          // Set new old position
          lastx=thisx;
          lasty=thisy;
        }
        else
          vtkInteractorStyleTrackballCamera::OnMouseMove();
      }

      ///  Overwrite keyboard callback
      virtual void OnChar() 
      {
        // Get the key pressed
        std::string key = this->Interactor->GetKeySym();

        //        cout << key << endl;

        // disable some standard vtk keys
        if(key== "f")  {}

        // q -> abort
        else if(key == "q")
        {
          abort();
        }

        // Reset Camera
        else if(key == "r")
        {
          for (auto & subframe: this->frame->subframes)
          {
            if (subframe.renderer==this->CurrentRenderer)
            {
              frame->RTResetCamera(subframe);
            }
          }
          this->Interactor->Render();
        }

        // Toggle wireframe
        else if(key == "w")
        {
          for (auto &figure: this->frame->figures)
            if (this->frame->subframes[figure->framepos].renderer==this->CurrentRenderer)
            {
              figure->state.wireframe=!figure->state.wireframe;
              if (figure->state.wireframe)
                for (auto & actor: figure->actors)  actor->GetProperty()->SetRepresentationToWireframe();
              else
                for (auto&  actor: figure->actors)  actor->GetProperty()->SetRepresentationToSurface();
              this->Interactor->Render();
            }
        
        }

        // Write output to png
        else if (key=="p")
        {

          // Generate file name using current time
          auto now = std::chrono::system_clock::now();
          auto time = std::chrono::system_clock::to_time_t(now);
          std::tm * ttm = localtime(&time);
          char time_str[] = "yyyy-mm-ddTHH:MM:SS ";
          strftime(time_str, strlen(time_str), "%Y-%m-%dT%H:%M:%S", ttm);
          auto fname=frame->parameter.wintitle+"-"+time_str+".png";

          auto imgfilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
          imgfilter->SetInput(frame->window);
          imgfilter->Update();

          auto pngwriter = vtkSmartPointer<vtkPNGWriter>::New();
          pngwriter->SetInputConnection(imgfilter->GetOutputPort());
          pngwriter->SetFileName(fname.c_str());
          
          this->Interactor->Render();
          pngwriter->Write();

          cout << "Frame written to "<< fname << endl;
        }

        // Keys which toggle editing
        else if (key == "x" || key== "y" || key== "z" || key== "l" || key== "a")
        {      
          if (!this->edit_mode)
          {
            
            this->edit_mode=true;
            /// Create list of currently edited figures. These
            /// are those the same subframe which means that they have
            /// the same renderer
            for (auto &figure: frame->figures)
            {

              if (
                frame->subframes[figure->framepos].renderer==this->CurrentRenderer
                // &&(
                //   (key=="a" && figure->SubClassName()=="Quiver")
                //   ||
                //   (key=="x" && figure->SubClassName()=="SurfaceContour")
                //   ||
                //   (key=="y" && figure->SubClassName()=="SurfaceContour")
                //   ||
                //   (key=="z" && figure->SubClassName()=="SurfaceContour")
                //   ||
                //   (key=="l" && figure->SubClassName()=="SurfaceContour")
                //  )
                )
                this->edited_figures.push_back(figure);
            }
          } 
          
          /// Pass key to edited figures
          for (auto figure : this->edited_figures)
            figure->RTProcessKey(key);
          this->Interactor->Render();
        }


        // Emulate mouse move for all edited figures
        else if(key == "Left" || key== "Down")
        {
          if (this->edit_mode)
          {
            for (auto figure : this->edited_figures)
              figure->RTProcessMove(-1,-1);
            this->Interactor->Render();
          }
        }

        // Emulate mouse move for all edited figures
        else if(key == "Right" || key== "Up")
        {
          if (this->edit_mode)
          {
            for (auto figure : this->edited_figures)
              figure->RTProcessMove(1,1);
            this->Interactor->Render();
          }
        }

        // These are interaction keys in edit mode
        else if(key == "Return" || key=="BackSpace")
        {
          if (this->edit_mode)
          {
            for (auto figure : this->edited_figures)
              figure->RTProcessKey(key);
            this->Interactor->Render();
          }
        }

        // Escape ends edit mode
        else if(key == "Escape")
        {
          this->edit_mode=false;
          for (auto figure : this->edited_figures)
          {
            figure->RTProcessKey(key);
            figure->RTMessage("");
          }
          this->Interactor->Render();
          this->edited_figures.resize(0);
          this->Interactor->Render();
        }
        
        // Toggle some states independent of edit mode
        else if(key == "i" || key== "L" || key == "e" || key == "s")
        {
          for (auto &figure: frame->figures)
            if (frame->subframes[figure->framepos].renderer==this->CurrentRenderer)
              figure->RTProcessKey(key);

          this->Interactor->Render();
        }

        // Block/unblock calculation
        else if (key=="space")
        {
          frame->mainthread->communication_blocked=!frame->mainthread->communication_blocked;
        }
        
        else if (key=="asterisk")
        {
          if (!frame->single_subframe_view)
            for(int i=0;i<this->frame->subframes.size();i++)
            {
              if (this->frame->subframes[i].renderer==this->CurrentRenderer)
                this->frame->visible_subframe=i;
            }
          
          frame->single_subframe_view=!frame->single_subframe_view;
          frame->RTSetSingleView(frame->single_subframe_view);
          this->Interactor->Render();
        }


        else if (frame->single_subframe_view && key=="Next")
        {
          frame->RTSetVisibleSubFrame(frame->visible_subframe-1,true);
          this->Interactor->Render();
        }

        else if (frame->single_subframe_view && key=="Prior")
        {
          frame->RTSetVisibleSubFrame(frame->visible_subframe+1,true);
          this->Interactor->Render();
        }


        
        // Print help string
        else if(key == "h" or key == "question")
        {
          cout << Frame::KeyboardHelp;
        }

        // Pass other keys to base
        else
        {
          vtkInteractorStyleTrackballCamera::OnChar();
        }
      }
    };
  


    ///////////////////////////////////////////////////////////////////
    /// Timer callback handling communication with render thread
    class MyTimerCallback : public vtkCommand
    {
    public:
      // Main thread to interact with
      MainThread* mainthread;
      
      
      vtkSmartPointer<vtkRenderWindowInteractor> Interactor=0;
    
      static MyTimerCallback *New()    {return new MyTimerCallback;}
    
      
      virtual void Execute(
        vtkObject *vtkNotUsed(caller),
        unsigned long eventId,
        void *vtkNotUsed(callData)
        )
      {
      

        if (this->mainthread->communication_blocked) return;
      
        if (
          vtkCommand::TimerEvent == eventId  // Check if timer event
          && this->mainthread->cmd!=Communicator::Command::Empty  // Check if command has been given
          )
        {        


          // Lock mutex
          if (this->mainthread->running_multithreaded)
            std::unique_lock<std::mutex> lock(this->mainthread->mutex);

          // Command dispatch
          switch(mainthread->cmd)
          {
          
           // Add frame to main thread
          case Communicator::Command::MainThreadAddFrame:
          {
            mainthread->RTAddFrame(mainthread, mainthread->iframe);
          }
          break;

          // Remove frame from mainthread
          case Communicator::Command::MainThreadRemoveFrame:
          {

            auto frame=mainthread->framemap[mainthread->iframe];
            for (auto & subframe: frame->subframes)
              subframe.renderer->RemoveAllViewProps();

            frame->window->Finalize();

            mainthread->framemap.erase(frame->number_in_frame_list);

          }
          break;


          // Add actors from figures to renderer
          case Communicator::Command::MainThreadShow:
          {
            for (auto & framepair: mainthread->framemap)
              framepair.second->RTAddFigures();
            for (auto & framepair: mainthread->framemap)
              framepair.second->window->Render();
            //this->Interactor->Render();
          }
          break;
        
          // Write picture to file
          case Communicator::Command::FrameDump:
          {
            auto frame=mainthread->framemap[mainthread->iframe];
            auto imgfilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
            auto pngwriter = vtkSmartPointer<vtkPNGWriter>::New();
          
            pngwriter->SetInputConnection(imgfilter->GetOutputPort());
            pngwriter->SetFileName(frame->parameter.filename.c_str());
          
            imgfilter->SetInput(frame->window);
            imgfilter->Update();
          
            this->Interactor->Render();
            pngwriter->Write();
          }
          break;


          case Communicator::Command::FrameLayout:
          {
            auto frame=mainthread->framemap[mainthread->iframe];
            frame->RTSetLayout(frame->parameter.nvpx,frame->parameter.nvpy); 
            frame->RTResetRenderers(false);
          }
          
          // Set frame size
          case Communicator::Command::FrameSize:
          {
            auto frame=mainthread->framemap[mainthread->iframe];
            frame->window->SetSize(frame->parameter.winsize_x, frame->parameter.winsize_y);
          }
          break;
        
          // Set frame size
          case Communicator::Command::FrameVisibleSubFrame:
          {
            auto frame=mainthread->framemap[mainthread->iframe];
            frame->RTSetVisibleSubFrame(frame->parameter.visible_subframe,false);
          }
          break;
        
          case Communicator::Command::FrameSingleView:
          {
            auto frame=mainthread->framemap[mainthread->iframe];
            frame->RTSetSingleView(frame->parameter.single_subframe_view);
          }
          break;
        


          // Set frame position
          case Communicator::Command::FramePosition:
          {
            auto frame=mainthread->framemap[mainthread->iframe];
            frame->window->SetPosition(frame->parameter.winposition_x, frame->parameter.winposition_y);
          }
          break;

          // Set frame title
          case Communicator::Command::FrameTitle:
          {
            auto frame=mainthread->framemap[mainthread->iframe];
            frame->RTInit();
            frame->title_actor->SetText(7,frame->parameter.frametitle.c_str());
            frame->title_actor->Modified();
                                      
          }
          break;

          // Set window title
          case Communicator::Command::WindowTitle:
          {
            auto frame=mainthread->framemap[mainthread->iframe];
            frame->window->SetWindowName(frame->parameter.wintitle.c_str());
          }
          break;

          // Link camera to other
          case Communicator::Command::FrameLinkCamera:
          {
            auto frame=mainthread->framemap[mainthread->iframe];
            auto renderer=mainthread->framemap[mainthread->iframe]->subframes[frame->parameter.camlinkthisframepos].renderer;
            auto lrenderer=mainthread->framemap[frame->parameter.camlinkframenum]->subframes[frame->parameter.camlinkframepos].renderer;
            renderer->SetActiveCamera(lrenderer->GetActiveCamera());
          }
          break;

          // Terminate
          case Communicator::Command::MainThreadTerminate:
          {

            for (auto & framepair: mainthread->framemap)
              framepair.second->window->Finalize();

            mainthread->framemap.clear();

            this->Interactor->TerminateApp();

            mainthread->running_multithreaded=false;

            mainthread->condition_variable.notify_all();

            return;
          }
          break;
        
          default:;
          }
        
          // Clear command 
          mainthread->cmd=Communicator::Command::Empty;
        
          // Notify that command was exeuted
          if (mainthread->running_multithreaded)
            // 
            mainthread->condition_variable.notify_all();
          else
            // dirty trick to unblock event loop.
            // hopefully works if multitreading does not
            mainthread->interactor->TerminateApp();
        
        }
      }
    };


    void MainThread::RTAddFrame(MainThread* mainthread, int iframe)
    {
      auto frame=mainthread->framemap[iframe];
      frame->window = vtkSmartPointer<vtkRenderWindow>::New();

      if (mainthread->double_buffering)
        frame->window->DoubleBufferOn();
      else
        frame->window->DoubleBufferOff();

      frame->window->SetSize(frame->parameter.winsize_x, frame->parameter.winsize_y);
      frame->window->SetPosition(frame->parameter.winposition_x, frame->parameter.winposition_y);
      
      frame->RTResetRenderers(true);

      frame->RTInit();
      frame->title_subframe.renderer->AddActor(frame->title_actor);
      frame->title_actor->SetText(7,frame->parameter.frametitle.c_str());

    }

    void MainThread::PrepareRenderThread(MainThread* mainthread)
    {
      RTAddFrame(mainthread,0);
      auto frame=mainthread->framemap[0];

      mainthread->interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
      auto style =  vtkSmartPointer<MyInteractorStyle>::New();
      style->SetFrame(frame);
      mainthread->interactor->SetInteractorStyle(style);
    
      mainthread->interactor->SetRenderWindow(frame->window);

      auto callback =  vtkSmartPointer<MyTimerCallback>::New();
      callback->Interactor=mainthread->interactor;
      callback->mainthread=mainthread;
      mainthread->interactor->AddObserver(vtkCommand::TimerEvent,callback);

      mainthread->interactor->Initialize();

      mainthread->interactor->CreateRepeatingTimer(mainthread->timer_interval);
    }
  
    void MainThread::RenderThread(MainThread* mainthread)
    {

      MainThread::PrepareRenderThread(mainthread);
      mainthread->running_multithreaded=true;

      mainthread->interactor->Start();
      mainthread->running_multithreaded=false;
      mainthread->condition_variable.notify_all();
    
      mainthread->interactor->SetRenderWindow(0);
      mainthread->interactor->TerminateApp();
      mainthread->running_multithreaded=false;
                                     
      //window->Finalize();
    }

    void MainThread::Start(void)
    {
      if (connection_open)
        this->thread=std::make_shared<std::thread>(CommunicatorThread,this);
      else
        this->thread=std::make_shared<std::thread>(RenderThread,this);

      do
      {
        std::this_thread::sleep_for (std::chrono::milliseconds(10));
      }
      while (!this->running_multithreaded);
    }


    ////////////////////////////////////////////////////////////////
    /// Server communication 
    void  MainThread::PrepareCommunicatorThread(MainThread* mainthread)
    {
      mainthread->cmd=Communicator::Command::MainThreadAddFrame;

    }

    void  MainThread::CommunicatorThreadCallback(MainThread* mainthread)
    {
      if (mainthread->cmd!=Communicator::Command::Empty)
      {      
        
        // Lock mutex
        if (mainthread->running_multithreaded)
          std::unique_lock<std::mutex> lock(mainthread->mutex);

        if (mainthread->debug_level>0) 
          cout << "s cmd: " << static_cast<int>(mainthread->cmd) << " frame: " <<mainthread->iframe<< endl;

        mainthread->communicator->SendCommand(mainthread->cmd);
        mainthread->communicator->SendInt(mainthread->iframe);

        // Command dispatch
        switch(mainthread->cmd)
        {
          
        case Communicator::Command::MainThreadAddFrame:
        {

        }
        break;

        case Communicator::Command::FrameLayout:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          mainthread->communicator->SendInt(frame->parameter.nvpx);
          mainthread->communicator->SendInt(frame->parameter.nvpy);
          frame->nvpx=frame->parameter.nvpx;
          frame->nvpy=frame->parameter.nvpy;
        }

        case Communicator::Command::MainThreadRemoveFrame:
        {

        }
        break;

        case Communicator::Command::MainThreadShow:
        {
          for (auto framepair: mainthread->framemap)
            for (auto & figure: framepair.second->figures)
            {
              figure->SetRange();
              figure->ServerRTSendData(mainthread->communicator);
            }
        }
        break;
        
        case Communicator::Command::FrameDump:
        {
          
          auto frame=mainthread->framemap[mainthread->iframe];
          mainthread->communicator->SendString(frame->parameter.filename);
        }
        break;
        
        case Communicator::Command::FrameSize:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          mainthread->communicator->SendInt(frame->parameter.winsize_x);
          mainthread->communicator->SendInt(frame->parameter.winsize_y);
        }
        break;

        case Communicator::Command::FramePosition:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          mainthread->communicator->SendInt(frame->parameter.winposition_x);
          mainthread->communicator->SendInt(frame->parameter.winposition_y);
        }
        break;

        case Communicator::Command::FrameTitle:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          mainthread->communicator->SendString(frame->parameter.frametitle);
        }
        break;


        case Communicator::Command::WindowTitle:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          mainthread->communicator->SendString(frame->parameter.wintitle);
        }
        break;


        case Communicator::Command::FrameAddFigure:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          auto figure=frame->figures.back();
          mainthread->communicator->SendString(figure->SubClassName());
          mainthread->communicator->SendInt(figure->framepos);
        }
        break;


        case Communicator::Command::FrameLinkCamera:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          mainthread->communicator->SendInt(frame->parameter.camlinkthisframepos);
          mainthread->communicator->SendInt(frame->parameter.camlinkframenum);
          mainthread->communicator->SendInt(frame->parameter.camlinkframepos);
        }
        break;

        case Communicator::Command::Clear:
        {
          // for (auto & figure: frame->figures)
          // {
            
          //   //frame->communicator->SendCommand(vtkfig::Command::FrameShow);
          // }
          
        }
        break;
        
        case Communicator::Command::MainThreadTerminate:
          // Close window and terminate
        {

          if (mainthread->debug_level>0)
            cout << "s term" << endl;
          mainthread->framemap.clear();
          // Notify that command was exeuted
          mainthread->condition_variable.notify_all();
          mainthread->running_multithreaded=false;
          return;
        }
        break;
        
        default:
        {
          cout << "s cmd: " << static_cast<int>(mainthread->cmd) << endl;
          throw std::runtime_error("wrong command on server");
        }
        
        break;
        }
        
        // Clear command
        mainthread->cmd=Communicator::Command::Empty;
        
        // Notify that command was exeuted
        if (mainthread->running_multithreaded)
          mainthread->condition_variable.notify_all();
      }

    }

    void  MainThread::CommunicatorThread(MainThread* mainthread)
    {
      mainthread->running_multithreaded=true;
      PrepareCommunicatorThread(mainthread);
      while(1)
      {
        std::this_thread::sleep_for (std::chrono::milliseconds(5));
        MainThread::CommunicatorThreadCallback(mainthread);
      }
      mainthread->running_multithreaded=false;
    }
  
 
  
  }

}
