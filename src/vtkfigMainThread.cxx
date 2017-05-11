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



#include "vtkfigFrame.h"
#include "vtkfigFigure.h"
#include "vtkfigMainThread.h"


namespace vtkfig
{
  MainThread::MainThread()
  {
    int wtime=5;
    char* port_string=getenv("VTKFIG_PORT_NUMBER");
    char* wtime_string=getenv("VTKFIG_WAIT_SECONDS");
    
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
  
  std::shared_ptr<Frame>  MainThread::AddFrame(int nrow, int ncol)
  {
    auto frame=std::make_shared<Frame>(nrow,ncol);
    frame->mainthread=shared_from_this();
    frame->framenum=lastframenum;
    framemap[lastframenum++]=frame;
    if (lastframenum==1)
    {
      Start();
    }
    else
    {
      SendCommand(frame->framenum, "AddFrame", MainThread::Command::AddFrame);
    }
    return frame;
  }


  void MainThread::Show()
  {
    SendCommand(-1, "Show", MainThread::Command::Show);
  }


  void MainThread::Interact()
  {
    if (!this->thread_alive)
      throw std::runtime_error("Interact: render thread is dead");

    this->communication_blocked=true;
    do
    {
      std::this_thread::sleep_for (std::chrono::milliseconds(10));
    }
    while (this->communication_blocked);

  }

  ////////////////////////////////////////////////////////////////
  /// Communication with render thread

  void MainThread::SendCommand(int framenum, const std::string from, MainThread::Command cmd)
  {
    if (!this->thread_alive)
      throw std::runtime_error(from+" : render thread is dead.");

    this->cmd=cmd;
    this->iframe=framenum;
    std::unique_lock<std::mutex> lock(this->mtx);
    this->cv.wait(lock);
  }

  void MainThread::Terminate(void)
  {
    SendCommand(-1,"Terminate",MainThread::Command::Terminate);
  }

  
  ////////////////////////////////////////////////////////////////
  /// Rendering 
  
  class InteractorStyleTrackballCamera : public vtkInteractorStyleTrackballCamera
  {
  public:
    std::shared_ptr<Frame> frame;
    
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
      }

      else if(key == "w")
      {

        frame->wireframe=!frame->wireframe;

        if (frame->wireframe)
        {
          for (auto &figure: frame->figures)
            for (auto & actor: figure->actors)  actor->GetProperty()->SetRepresentationToWireframe();
          
        }
        else
        {
          for (auto & figure: frame->figures)
            for (auto&  actor: figure->actors)  actor->GetProperty()->SetRepresentationToSurface();
        }
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
    
    std::shared_ptr<MainThread> mainthread;
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
        && mainthread->cmd!=MainThread::Command::None  // Check if command has been given
        )
      {
        
        // Lock mutex
        std::unique_lock<std::mutex> lock(mainthread->mtx);
        
        // Command dispatch
        switch(mainthread->cmd)
        {
          
        case MainThread::Command::AddFrame:
        {

          mainthread->RTAddFrame(mainthread, mainthread->iframe);
          auto frame=mainthread->framemap[mainthread->iframe];
        }
       break;


          // Add actors to renderer
        case MainThread::Command::Show:
        {
          for (auto & framepair: mainthread->framemap)
            for (auto & figure: framepair.second->figures)
            {
              auto &renderer=framepair.second->subframes[figure->framepos].renderer;
              
              if (figure->IsEmpty()  || renderer->GetActors()->GetNumberOfItems()==0)
              {
                // This allows clear figure to work
                renderer->RemoveAllViewProps();
                
                figure->RTBuild();
                
                for (auto & actor: figure->actors) 
                  renderer->AddActor(actor);
                
                for (auto & actor: figure->actors2d) 
                  renderer->AddActor(actor);
                
                figure->RTSetInteractor(interactor,renderer);
                
              }
              figure->RTUpdateActors();
              renderer->SetBackground(figure->bgcolor[0],
                                      figure->bgcolor[1],
                                      figure->bgcolor[2]);
            }
          interactor->Render();
        }
        break;
        
          // Write picture to file
        case MainThread::Command::Dump:
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

        

        
          // Close window and terminate
        case MainThread::Command::Resize:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          frame->window->SetSize(frame->win_x, frame->win_y);
        }
        break;
        
        case MainThread::Command::Reposition:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          frame->window->SetPosition(frame->pos_x, frame->pos_y);
        }
        break;
        
          // Close window and terminate
        case MainThread::Command::Terminate:
        {

          for (auto & framepair: mainthread->framemap)
            framepair.second->window->Finalize();
          interactor->TerminateApp();
        }
        break;
        
        default:;
        }
        
        // Clear command
        mainthread->cmd=MainThread::Command::None;
        
        // Notify that command was exeuted
        mainthread->cv.notify_all();

        for (auto & framepair: mainthread->framemap)
          if (framepair.first)
            framepair.second->window->Render();

      }
    }
  };

  void MainThread::RTAddFrame(std::shared_ptr<MainThread> mainthread, int iframe)
  {
    auto frame=mainthread->framemap[iframe];
    frame->window = vtkSmartPointer<vtkRenderWindow>::New();
    frame->window->SetSize(frame->win_x, frame->win_y);
    frame->window->SetPosition(frame->pos_x, frame->pos_y);

    for (auto & subframe : frame->subframes)
    {
      /*
        For Subplots see http://public.kitware.com/pipermail/vtkusers/2009-October/054195.html
        renderer2->SetActiveCamera( renderer1->GetActiveCamera() );
        
      */
      
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
  
  void MainThread::RenderThread(std::shared_ptr<MainThread> mainthread)
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
                                      
    
    //window->Finalize();
  }

  void MainThread::Start(void)
  {
    if (connection_open)
      this->thread=std::make_shared<std::thread>(CommunicatorThread,shared_from_this());
    else
      this->thread=std::make_shared<std::thread>(RenderThread,shared_from_this());

    do
    {
      std::this_thread::sleep_for (std::chrono::milliseconds(10));
    }
    while (!this->thread_alive);
  }


  ////////////////////////////////////////////////////////////////
  /// Server communication 

  void  MainThread::CommunicatorThread( std::shared_ptr<MainThread> mainthread)
  {
    mainthread->thread_alive=true;

    auto frame=mainthread->framemap[0];
    mainthread->communicator->SendInt(-1);
    mainthread->communicator->SendCommand(vtkfig::Command::AddFrame);
    mainthread->communicator->SendInt(frame->nrow);
    mainthread->communicator->SendInt(frame->ncol);

    while(1)
    {
      std::this_thread::sleep_for (std::chrono::milliseconds(5));

      if (mainthread->cmd!=MainThread::Command::None)
      {      
        
        // Lock mutex
        std::unique_lock<std::mutex> lock(mainthread->mtx);


        mainthread->communicator->SendInt(mainthread->iframe);

        // Command dispatch
        switch(mainthread->cmd)
        {
          
        case MainThread::Command::AddFrame:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          mainthread->communicator->SendCommand(vtkfig::Command::AddFrame);
          mainthread->communicator->SendInt(frame->nrow);
          mainthread->communicator->SendInt(frame->ncol);

        }
        break;

        case MainThread::Command::Show:
        {
          mainthread->communicator->SendCommand(vtkfig::Command::MainThreadShow);
          for (auto framepair: mainthread->framemap)
            for (auto & figure: framepair.second->figures)
            {
              figure->ServerRTSend(mainthread->communicator);
            }
        }
        break;
        
        case MainThread::Command::Dump:
        {
          
          auto frame=mainthread->framemap[mainthread->iframe];
          mainthread->communicator->SendCommand(vtkfig::Command::FrameDump);
          mainthread->communicator->SendString(frame->fname);
        }
        break;
        
        case MainThread::Command::Resize:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          mainthread->communicator->SendCommand(vtkfig::Command::FrameResize);
          mainthread->communicator->SendInt(frame->win_x);
          mainthread->communicator->SendInt(frame->win_y);
        }
        break;

        case MainThread::Command::Reposition:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          mainthread->communicator->SendCommand(vtkfig::Command::FrameReposition);
          mainthread->communicator->SendInt(frame->pos_x);
          mainthread->communicator->SendInt(frame->pos_y);
        }
        break;

        case MainThread::Command::AddFigure:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          auto figure=frame->figures.back();
          mainthread->communicator->SendCommand(vtkfig::Command::AddFigure);
          mainthread->communicator->SendString(figure->SubClassName());
          mainthread->communicator->SendInt(frame->row(figure->framepos));
          mainthread->communicator->SendInt(frame->col(figure->framepos));
        }
        break;

        case MainThread::Command::Clear:
        {
          // for (auto & figure: frame->figures)
          // {
            
          //   //frame->communicator->SendCommand(vtkfig::Command::FrameShow);
          // }
          
        }
        break;
        
        case MainThread::Command::Terminate:
          // Close window and terminate
        {
          //mainthread->communicator->SendCommand(vtkfig::Command::FrameDelete);
        }
        break;
        
        default:
          break;
        }
        
        // Clear command
        mainthread->cmd=MainThread::Command::None;
        
        // Notify that command was exeuted
        mainthread->cv.notify_all();
      }
    }
  }
  
 
  
}
