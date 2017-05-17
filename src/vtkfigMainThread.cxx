#include <stdexcept>

#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkWindowToImageFilter.h"
#include "vtkPNGWriter.h"
#include "vtkCommand.h"
#include "vtkCamera.h"
#include "vtkProperty.h"
#include "vtkProperty2D.h"
#include "vtkObjectBase.h"
#include "vtkPropCollection.h"



#include "vtkfigFrame.h"
#include "vtkfigFigure.h"
#include "vtkfigMainThread.h"


namespace vtkfig
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
    char* port_string=getenv("VTKFIG_PORT_NUMBER");
    char* wtime_string=getenv("VTKFIG_WAIT_SECONDS");
    char *debug_string=getenv("VTKFIG_DEBUG");
    if (debug_string!=0)
      debug_level=atoi(debug_string);

    if (wtime_string!=0)
      wtime=atoi(wtime_string);
    
    
    if (port_string!=0)
    {
      int port=atoi(port_string);
      if (port<=0) 
        throw std::runtime_error("Invalid port number for server");
      OpenConnection(port,wtime);
    }

  }


  void MainThread::OpenConnection(int port, int wtime)
  {
    if (connection_open) return;

    cout << "Server start listening on port "<< port << endl;
    communicator=vtkSmartPointer<vtkfig::Communicator>::New();
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
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if (this->thread_alive)
      Terminate();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    this->thread->join();
    framemap.clear();
  }

  
  void  MainThread::AddFrame(Frame*frame)
  {
    frame->mainthread=this;
    frame->framenum=lastframenum;
    this->iframe=lastframenum;
    framemap[lastframenum++]=frame;
    if (lastframenum==1)
    {
      Start();
    }
    else
    {
      SendCommand(frame->framenum, "AddFrame", Communicator::Command::MainThreadAddFrame);
    }
  }


  void MainThread::RemoveFrame(Frame *frame)
  {
    assert(frame==framemap[frame->framenum]);
    SendCommand(frame->framenum, "RemoveFrame", Communicator::Command::MainThreadRemoveFrame);
    frame->framenum=-1;
  }

  void MainThread::Show()
  {
    SendCommand(-1, "Show", Communicator::Command::MainThreadShow);
  }


  void MainThread::Interact()
  {
    if (!this->thread_alive)
      throw std::runtime_error("Interact: render thread is dead");

    SendCommand(-1, "Show", Communicator::Command::MainThreadShow);

    this->communication_blocked=true;
    do
    {
      std::this_thread::sleep_for (std::chrono::milliseconds(10));
    }
    while (this->communication_blocked);

  }

  ////////////////////////////////////////////////////////////////
  /// Communication with render thread

  void MainThread::SendCommand(int framenum, const std::string from, Communicator::Command cmd)
  {
    if (!this->thread_alive)
      throw std::runtime_error(from+" : render thread is dead.");
    if (debug_level>0)
      cout << "mt " << from << " " << framenum << endl;
    this->cmd=cmd;
    this->iframe=framenum;
    std::unique_lock<std::mutex> lock(this->mtx);
    this->cv.wait(lock);
  }

  void MainThread::Terminate(void)
  {
    SendCommand(-1,"Terminate",Communicator::Command::MainThreadTerminate);
  }

  
  ////////////////////////////////////////////////////////////////
  /// Rendering 
  
  class InteractorStyleTrackballCamera : public vtkInteractorStyleTrackballCamera
  {
  public:
    Frame* frame;
    
    static InteractorStyleTrackballCamera* New()
    {
      return new InteractorStyleTrackballCamera();
    }
    InteractorStyleTrackballCamera(): vtkInteractorStyleTrackballCamera() {};
    
    
    virtual void OnChar() 
    {
      // Get the keypress
      vtkRenderWindowInteractor *interactor = this->Interactor;

      std::string key = interactor->GetKeySym();
      
      if(key == "e" || key== "q" || key== "f")  {}

      else if(key == "r")
      {
        for (auto & subframe: frame->subframes)
        {
          subframe.renderer->GetActiveCamera()->SetPosition(subframe.default_camera_position);
          subframe.renderer->GetActiveCamera()->SetFocalPoint(subframe.default_camera_focal_point);
          subframe.renderer->GetActiveCamera()->OrthogonalizeViewUp();
          subframe.renderer->GetActiveCamera()->SetRoll(0);
        }
        interactor->Render();
      }

      else if(key == "w")
      {
        /// get figure number from mouse position
        for (auto &figure: frame->figures)
        { 
          figure->state.wireframe=!figure->state.wireframe;
          if (figure->state.wireframe)
            for (auto & actor: figure->actors)  actor->GetProperty()->SetRepresentationToWireframe();
          else
            for (auto&  actor: figure->actors)  actor->GetProperty()->SetRepresentationToSurface();
        }
        interactor->Render();
      }

      else if(key == "l")
      {
        /// get figure number from mouse position
        for (auto &figure: frame->figures)
        {
          figure->state.num_contours=(figure->state.num_contours+1)%figure->state.max_num_contours;
          figure->SetVMinMax(figure->state.real_vmin, figure->state.real_vmax);
        }
        interactor->Render();
      }

      
      else if (key=="space")
      {
        frame->mainthread->communication_blocked=!frame->mainthread->communication_blocked;
      }

      else if(key == "h" or key == "?")
      {
        cout << Frame::keyboard_help;
      }

      else
      {
        vtkInteractorStyleTrackballCamera::OnChar();
      }
    }
  };
  
 
  class TimerCallback : public vtkCommand
  {
  public:
    
    MainThread* mainthread;
    vtkSmartPointer<vtkRenderWindowInteractor> interactor=0;
    
    static TimerCallback *New()    {return new TimerCallback;}
    
    virtual void Execute(
      vtkObject *vtkNotUsed(caller),
      unsigned long eventId,
      void *vtkNotUsed(callData)
      )
    {
      
      if (mainthread->communication_blocked) return;
      
      if (
        vtkCommand::TimerEvent == eventId  // Check if timer event
        && mainthread->cmd!=Communicator::Command::Empty  // Check if command has been given
        )
      {
        
        // Lock mutex
        std::unique_lock<std::mutex> lock(mainthread->mtx);
        
        // Command dispatch
        switch(mainthread->cmd)
        {
          
        case Communicator::Command::MainThreadAddFrame:
        {

          mainthread->RTAddFrame(mainthread, mainthread->iframe);
          auto frame=mainthread->framemap[mainthread->iframe];
        }
       break;

        case Communicator::Command::MainThreadRemoveFrame:
        {

          auto frame=mainthread->framemap[mainthread->iframe];
          for (auto & subframe: frame->subframes)
            subframe.renderer->RemoveAllViewProps();
          frame->window->Finalize();
          mainthread->framemap.erase(frame->framenum);
        }
       break;


          // Add actors to renderer
        case Communicator::Command::MainThreadShow:
        {
          for (auto & framepair: mainthread->framemap)
            for (auto & figure: framepair.second->figures)
            {
              auto &renderer=framepair.second->subframes[figure->framepos].renderer;
              auto &window=framepair.second->window;
              if (figure->IsEmpty()  || renderer->GetActors()->GetNumberOfItems()==0)
              {
                figure->RTBuild(window,interactor,renderer);
                
                for (auto & actor: figure->actors) 
                  renderer->AddActor(actor);
  
                for (auto & actor: figure->ctxactors) 
                  renderer->AddActor(actor);
  
                for (auto & actor: figure->actors2d) 
                  renderer->AddActor(actor);
                
              }
              figure->RTUpdateActors();
              renderer->SetBackground(figure->bgcolor[0],
                                      figure->bgcolor[1],
                                      figure->bgcolor[2]);
            }
          for (auto & framepair: mainthread->framemap)
              framepair.second->window->Render();
//          interactor->Render();
        }
        break;
        
          // Write picture to file
        case Communicator::Command::FrameDump:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          auto imgfilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
          auto pngwriter = vtkSmartPointer<vtkPNGWriter>::New();
          
          pngwriter->SetInputConnection(imgfilter->GetOutputPort());
          pngwriter->SetFileName(frame->fname.c_str());
          
          imgfilter->SetInput(frame->window);
          imgfilter->Update();
          
          interactor->Render();
          pngwriter->Write();
        }
        break;

        

        
        case Communicator::Command::FrameSize:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          frame->window->SetSize(frame->win_x, frame->win_y);
        }
        break;
        
        case Communicator::Command::FramePosition:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          frame->window->SetPosition(frame->pos_x, frame->pos_y);
        }
        break;

        case Communicator::Command::FrameLinkCamera:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          auto renderer=mainthread->framemap[mainthread->iframe]->subframes[frame->camlinkthisframepos].renderer;
          auto lrenderer=mainthread->framemap[frame->camlinkframenum]->subframes[frame->camlinkframepos].renderer;
          renderer->SetActiveCamera(lrenderer->GetActiveCamera());
        }
        break;



        
        case Communicator::Command::MainThreadTerminate:
        {

          for (auto & framepair: mainthread->framemap)
            framepair.second->window->Finalize();
          mainthread->framemap.clear();
          interactor->TerminateApp();
          mainthread->thread_alive=false;
          mainthread->cv.notify_all();
          return;
        }
        break;
        
        default:;
        }
        
        // Clear command
        mainthread->cmd=Communicator::Command::Empty;
        
        // Notify that command was exeuted
        mainthread->cv.notify_all();
        
      }
    }
  };


  void MainThread::RTAddFrame(MainThread* mainthread, int iframe)
  {
    auto frame=mainthread->framemap[iframe];
    frame->window = vtkSmartPointer<vtkRenderWindow>::New();
    frame->window->DoubleBufferOff();
    frame->window->SetSize(frame->win_x, frame->win_y);
    frame->window->SetPosition(frame->pos_x, frame->pos_y);

    for (auto & subframe : frame->subframes)
    {
      
      {
        auto renderer = vtkSmartPointer<vtkRenderer>::New();
        subframe.renderer=renderer;
        renderer->SetViewport(subframe.viewport);
        renderer->SetBackground(1., 1., 1.);
        //      renderer->SetUseHiddenLineRemoval(1);


        renderer->GetActiveCamera()->SetPosition(subframe.default_camera_position);
        renderer->GetActiveCamera()->SetFocalPoint(subframe.default_camera_focal_point);
        renderer->GetActiveCamera()->OrthogonalizeViewUp();
        frame->window->AddRenderer(renderer);
        
      }
    }
  }
  
  void MainThread::RenderThread(MainThread* mainthread)
  {

    RTAddFrame(mainthread,0);
    auto frame=mainthread->framemap[0];

    auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    auto style =  vtkSmartPointer<InteractorStyleTrackballCamera>::New();
    style->frame=frame;
    interactor->SetInteractorStyle(style);
    
    interactor->SetRenderWindow(frame->window);

    auto callback =  vtkSmartPointer<TimerCallback>::New();
    callback->interactor=interactor;
    callback->mainthread=mainthread;
    interactor->AddObserver(vtkCommand::TimerEvent,callback);




    interactor->Initialize();
    int timerId = interactor->CreateRepeatingTimer(10);
  
    mainthread->thread_alive=true;
    interactor->Start();
    mainthread->thread_alive=false;
    mainthread->cv.notify_all();
    
    interactor->SetRenderWindow(0);
    interactor->TerminateApp();
    mainthread->thread_alive=false;
                                      
    
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
    while (!this->thread_alive);
  }


  ////////////////////////////////////////////////////////////////
  /// Server communication 

  void  MainThread::CommunicatorThread(MainThread* mainthread)
  {
    mainthread->thread_alive=true;


    mainthread->cmd=Communicator::Command::MainThreadAddFrame;

    while(1)
    {
      std::this_thread::sleep_for (std::chrono::milliseconds(5));
      if (mainthread->cmd!=Communicator::Command::Empty)
      {      
        
        // Lock mutex
        std::unique_lock<std::mutex> lock(mainthread->mtx);

        if (mainthread->debug_level>0) 
          cout << "s cmd: " << static_cast<int>(mainthread->cmd) << " frame: " <<mainthread->iframe<< endl;

        mainthread->communicator->SendCommand(mainthread->cmd);
        mainthread->communicator->SendInt(mainthread->iframe);

        // Command dispatch
        switch(mainthread->cmd)
        {
          
        case Communicator::Command::MainThreadAddFrame:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          mainthread->communicator->SendInt(frame->nvpx);
          mainthread->communicator->SendInt(frame->nvpy);

        }
        break;

        case Communicator::Command::MainThreadRemoveFrame:
        {

        }
        break;

        case Communicator::Command::MainThreadShow:
        {
          for (auto framepair: mainthread->framemap)
            for (auto & figure: framepair.second->figures)
            {
              figure->ServerRTSend(mainthread->communicator);
            }
        }
        break;
        
        case Communicator::Command::FrameDump:
        {
          
          auto frame=mainthread->framemap[mainthread->iframe];
          mainthread->communicator->SendString(frame->fname);
        }
        break;
        
        case Communicator::Command::FrameSize:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          mainthread->communicator->SendInt(frame->win_x);
          mainthread->communicator->SendInt(frame->win_y);
        }
        break;

        case Communicator::Command::FramePosition:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          mainthread->communicator->SendInt(frame->pos_x);
          mainthread->communicator->SendInt(frame->pos_y);
        }
        break;


        case Communicator::Command::FrameAddFigure:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          auto figure=frame->figures.back();
          mainthread->communicator->SendString(figure->SubClassName());
          mainthread->communicator->SendInt(frame->ivpx(figure->framepos));
          mainthread->communicator->SendInt(frame->ivpy(figure->framepos));
        }
        break;


        case Communicator::Command::FrameLinkCamera:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          mainthread->communicator->SendInt(frame->camlinkthisframepos);
          mainthread->communicator->SendInt(frame->camlinkframenum);
          mainthread->communicator->SendInt(frame->camlinkframepos);
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
          mainthread->cv.notify_all();
          mainthread->thread_alive=false;
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
        mainthread->cv.notify_all();
      }
    }
    mainthread->thread_alive=false;
  }
  
 
  
}
