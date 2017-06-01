#include <stdexcept>
#include <chrono>

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
#include "config.h"


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
    char *multi_string=getenv("VTKFIG_MULTITHREADED");


    if (debug_string!=0)
      debug_level=atoi(debug_string);

    
#if CONFIG_APPLE
    try_running_multithreaded=false;
    if (multi_string!=0 && atoi(multi_string))
    {
    if (debug_level>0)
      cout << "overriding multithreading default (off) on APPLE" << endl;
    try_running_multithreaded=false;
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

  ////////////////////////////////////////////////////////////////
  /// Communication with render thread

  void MainThread::SendCommand(int number_in_frame_list, const std::string from, Communicator::Command cmd)
  {
    if (debug_level>0)
      cout << "mt " << from << " " << number_in_frame_list << endl;


    this->cmd=cmd;
    this->iframe=number_in_frame_list;
    if (running_multithreaded)
    {
      std::unique_lock<std::mutex> lock(this->mtx);
      this->cv.wait(lock);
    }
    else
      Update();
  }

  void MainThread::Terminate(void)
  {
    SendCommand(-1,"Terminate",Communicator::Command::MainThreadTerminate);
  }

  
  ////////////////////////////////////////////////////////////////
  /// Rendering 
  
  class MyInteractorStyle : public vtkInteractorStyleTrackballCamera
  {
  public:
    Frame* frame;
    
    static MyInteractorStyle* New()
    {
      return new MyInteractorStyle();
    }
    MyInteractorStyle(): vtkInteractorStyleTrackballCamera() {};



    std::vector<Figure*>edited_figures;
    bool edit_mode =false;
    bool bdown=false;

    int lastx,lasty;
    virtual void OnLeftButtonDown()
    {

      vtkRenderWindowInteractor *rwi = this->Interactor;
      if (edit_mode)
      {
        bdown=true;
        lastx=rwi->GetEventPosition()[0];
        lasty=rwi->GetEventPosition()[1];
      }
      else
        vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
    }

    virtual void OnLeftButtonUp()
    {
      if (bdown)
        bdown=false;
      else
        vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
    }

    virtual void OnMouseMove()
    {
      vtkRenderWindowInteractor *rwi = this->Interactor;
      if (bdown)
        {
          int thisx=rwi->GetEventPosition()[0];
          int thisy=rwi->GetEventPosition()[1];
          int dx=thisx-lastx;
          int dy=thisy-lasty;
          for (auto figure:edited_figures) figure->RTProcessMove(dx,dy);
          lastx=thisx;
          lasty=thisy;
          rwi->Render();
        }
      else
        vtkInteractorStyleTrackballCamera::OnMouseMove();
    }
    
    virtual void OnChar() 
    {
      // Get the keypress
      vtkRenderWindowInteractor *interactor = this->Interactor;


      std::string key = interactor->GetKeySym();
//      cout << key << endl;
      if(key == "e" ||  key== "f")  {}

      else if(key == "q")
      {
        abort();
      }
      else if(key == "r")
      {
        for (auto & subframe: frame->subframes)
        {
          if (subframe.renderer==this->CurrentRenderer)
          {
            subframe.renderer->GetActiveCamera()->SetPosition(subframe.default_camera_position);
            subframe.renderer->GetActiveCamera()->SetFocalPoint(subframe.default_camera_focal_point);
            subframe.renderer->GetActiveCamera()->OrthogonalizeViewUp();
            subframe.renderer->GetActiveCamera()->SetRoll(0);
            subframe.renderer->GetActiveCamera()->Zoom(subframe.default_camera_zoom);
            subframe.renderer->GetActiveCamera()->SetViewAngle(subframe.default_camera_view_angle);
          }
        }
        interactor->Render();
      }
      else if(key == "w")
      {
        for (auto &figure: frame->figures)
          if (frame->subframes[figure->framepos].renderer==this->CurrentRenderer)
          {
            figure->state.wireframe=!figure->state.wireframe;
            if (figure->state.wireframe)
              for (auto & actor: figure->actors)  actor->GetProperty()->SetRepresentationToWireframe();
            else
              for (auto&  actor: figure->actors)  actor->GetProperty()->SetRepresentationToSurface();
            interactor->Render();
          }
        
      }

      else if (key=="p")
      {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm * ttm = localtime(&time);
        char time_str[] = "yyyy-mm-ddTHH:MM:SS ";
        strftime(time_str, strlen(time_str), "%Y-%m-%dT%H:%M:%S", ttm);
        auto fname=frame->parameter.wintitle+"-"+time_str+".png";

        auto imgfilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
        auto pngwriter = vtkSmartPointer<vtkPNGWriter>::New();
          
        pngwriter->SetInputConnection(imgfilter->GetOutputPort());
        pngwriter->SetFileName(fname.c_str());
          
        imgfilter->SetInput(frame->window);
        imgfilter->Update();
          
        interactor->Render();
        pngwriter->Write();

        cout << "Frame written to "<< fname << endl;
      }


      else if (key == "x" || key== "y" || key== "z" || key== "l" || key== "a")
      {      
        if (!edit_mode)
        {
          
          for (auto &figure: frame->figures)
            if (frame->subframes[figure->framepos].renderer==this->CurrentRenderer)
              edited_figures.push_back(figure);
          edit_mode=true;
        } 
        for (auto figure : edited_figures)
          figure->RTProcessKey(key);
        interactor->Render();


      }
      else if(key == "Left" || key== "Down")
      {
        if (edit_mode)
        {
          for (auto figure : edited_figures)
            figure->RTProcessMove(-1,0);
          interactor->Render();
        }
      }
      else if(key == "Right" || key== "Up")
      {
        if (edit_mode)
        {
          for (auto figure : edited_figures)
            figure->RTProcessMove(1,0);
          interactor->Render();
        }
      }
      else if(key == "Return" || key=="BackSpace")
      {
        if (edit_mode)
        {
          for (auto figure : edited_figures)
            figure->RTProcessKey(key);
          interactor->Render();
        }
      }
      else if(key == "Escape")
      {
        edit_mode=false;
        for (auto figure : edited_figures)
        {
          figure->RTProcessKey(key);
          figure->RTMessage("");
        }
        interactor->Render();
        edited_figures.resize(0);
        
      }
      else if(key == "i" || key== "L")
      {
        for (auto &figure: frame->figures)
          if (frame->subframes[figure->framepos].renderer==this->CurrentRenderer)
          {
            figure->RTProcessKey(key);
          }
        interactor->Render();
      }
      else if (key=="space")
      {
        frame->mainthread->communication_blocked=!frame->mainthread->communication_blocked;
      }

      else if(key == "h" or key == "question")
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
        if (mainthread->running_multithreaded)
          std::unique_lock<std::mutex> lock(mainthread->mtx);
        
        // Command dispatch
        switch(mainthread->cmd)
        {
          
        case Communicator::Command::MainThreadAddFrame:
        {

          mainthread->RTAddFrame(mainthread, mainthread->iframe);
        }
       break;

        case Communicator::Command::MainThreadRemoveFrame:
        {

          auto frame=mainthread->framemap[mainthread->iframe];
          for (auto & subframe: frame->subframes)
            subframe.renderer->RemoveAllViewProps();
          frame->window->Finalize();
          mainthread->framemap.erase(frame->number_in_frame_list);
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
                figure->RTBuildVTKPipeline(window,interactor,renderer);

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
          pngwriter->SetFileName(frame->parameter.filename.c_str());
          
          imgfilter->SetInput(frame->window);
          imgfilter->Update();
          
          interactor->Render();
          pngwriter->Write();
        }
        break;

        

        
        case Communicator::Command::FrameSize:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          frame->window->SetSize(frame->parameter.winsize_x, frame->parameter.winsize_y);
        }
        break;
        
        case Communicator::Command::FramePosition:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          frame->window->SetPosition(frame->parameter.winposition_x, frame->parameter.winposition_y);
        }
        break;

        case Communicator::Command::FrameTitle:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          frame->title_actor->SetText(7,frame->parameter.frametitle.c_str());
          frame->title_actor->Modified();
                                      
        }
        break;

        case Communicator::Command::WindowTitle:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          frame->window->SetWindowName(frame->parameter.wintitle.c_str());
        }
        break;

        case Communicator::Command::FrameLinkCamera:
        {
          auto frame=mainthread->framemap[mainthread->iframe];
          auto renderer=mainthread->framemap[mainthread->iframe]->subframes[frame->parameter.camlinkthisframepos].renderer;
          auto lrenderer=mainthread->framemap[frame->parameter.camlinkframenum]->subframes[frame->parameter.camlinkframepos].renderer;
          renderer->SetActiveCamera(lrenderer->GetActiveCamera());
        }
        break;



        
        case Communicator::Command::MainThreadTerminate:
        {

          for (auto & framepair: mainthread->framemap)
            framepair.second->window->Finalize();
          mainthread->framemap.clear();
          interactor->TerminateApp();
          mainthread->running_multithreaded=false;
          mainthread->cv.notify_all();
          return;
        }
        break;
        
        default:;
        }
        
        // Clear command
        mainthread->cmd=Communicator::Command::Empty;
        
        // Notify that command was exeuted
        if (mainthread->running_multithreaded)
          mainthread->cv.notify_all();
        else
          mainthread->interactor->TerminateApp();
        
      }
    }
  };


  void MainThread::RTAddFrame(MainThread* mainthread, int iframe)
  {
    auto frame=mainthread->framemap[iframe];
    frame->window = vtkSmartPointer<vtkRenderWindow>::New();
    frame->window->DoubleBufferOff();
    frame->window->SetSize(frame->parameter.winsize_x, frame->parameter.winsize_y);
    frame->window->SetPosition(frame->parameter.winposition_x, frame->parameter.winposition_y);

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
        renderer->GetActiveCamera()->Zoom(subframe.default_camera_zoom);
        renderer->GetActiveCamera()->SetViewAngle(subframe.default_camera_view_angle);
        renderer->GetActiveCamera()->OrthogonalizeViewUp();
        frame->window->AddRenderer(renderer);
        
      }
    }

    auto & subframe=frame->subframes[frame->nvpx*frame->nvpy];
    subframe.renderer->AddActor(frame->title_actor);
    frame->title_actor->SetText(7,frame->parameter.frametitle.c_str());

  }


  void MainThread::PrepareRenderThread(MainThread* mainthread)
  {
    RTAddFrame(mainthread,0);
    auto frame=mainthread->framemap[0];

    mainthread->interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    auto style =  vtkSmartPointer<MyInteractorStyle>::New();
    style->frame=frame;
    mainthread->interactor->SetInteractorStyle(style);
    
    mainthread->interactor->SetRenderWindow(frame->window);

    auto callback =  vtkSmartPointer<TimerCallback>::New();
    callback->interactor=mainthread->interactor;
    callback->mainthread=mainthread;
    mainthread->interactor->AddObserver(vtkCommand::TimerEvent,callback);




    mainthread->interactor->Initialize();
    mainthread->interactor->CreateRepeatingTimer(10);
  }
  
  void MainThread::RenderThread(MainThread* mainthread)
  {

    MainThread::PrepareRenderThread(mainthread);
  
    mainthread->running_multithreaded=true;
    mainthread->interactor->Start();
    mainthread->running_multithreaded=false;
    mainthread->cv.notify_all();
    
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
          mainthread->communicator->SendInt(frame->ivpx(figure->framepos));
          mainthread->communicator->SendInt(frame->ivpy(figure->framepos));
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
          mainthread->cv.notify_all();
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
          mainthread->cv.notify_all();
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
