#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleImage.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkCamera.h"
#include "vtkProperty.h"
#include "vtkProperty2D.h"
#include "vtkCommand.h"
#include "vtkWindowToImageFilter.h"
#include "vtkPNGWriter.h"
#include "vtkObjectBase.h"



#include "vtkfigFrame.h"
#include "vtkfigFigure.h"

namespace vtkfig
{

  /////////////////////////////////////
  /// Public API 

  Frame::Frame(const int nrow, const int ncol):
  nrow(nrow),
    ncol(ncol)
  {
    framenum=lastframenum;
    Frame::frames[lastframenum++]=this;

    figures.clear();
    double dx= 1.0/(double)ncol;
    double dy= 1.0/(double)nrow;
    subframes.resize(nrow*ncol);
    
    double y=1.0;
    for (int irow=0 ;irow<nrow;irow++, y-=dy)
    {
      double x=0.0;
      for (int icol=0;icol<ncol;icol++, x+=dx)
      {
        int ipos=pos(irow,icol);
        
        assert(row(ipos)==irow);
        assert(col(ipos)==icol);
        
        
        auto & subframe=subframes[ipos];
        subframe.viewport[0]=x;
        subframe.viewport[1]=y-dy;
        subframe.viewport[2]=x+dx;
        subframe.viewport[3]=y;
      }
    }
    StartRenderThread();
  }



  Frame::Frame(vtkSmartPointer<vtkfig::Communicator> communicator,const int nrow, const int ncol):
    nrow(nrow),
    ncol(ncol)
  {
    framenum=lastframenum;
    Frame::frames[lastframenum++]=this;

    this->communicator=communicator;
    this->communicator->SendInt(-1);
    this->communicator->SendCommand(vtkfig::Command::NewFrame);
    this->communicator->SendInt(nrow);
    this->communicator->SendInt(ncol);
    StartCommunicatorThread();
  }

  void Frame::AddFigure(std::shared_ptr<Figure> fig, int irow, int icol)
  {
    assert(irow<this->nrow);
    assert(icol<this->ncol);

    this->figures.push_back(fig);
    int pos=this->pos(irow,icol);
    fig->framepos=pos;

    SendCommand("AddFigure", Frame::Command::AddFigure);
  }



  void Frame::Show()
  {
    SendCommand("Show", Frame::Command::Show);
  }


  void Frame::Interact()
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

  void Frame::Dump(std::string fname)
  {
    this->fname=fname;
    SendCommand("Dump", Frame::Command::Dump);
  }

  void Frame::Resize(int x, int y)
  {
    this->win_x=x;
    this->win_y=y;
    SendCommand("Resize", Frame::Command::Resize);
  }

  void Frame::Reposition(int x, int y)
  {
    this->win_x=x;
    this->win_y=y;
    SendCommand("Reposition", Frame::Command::Reposition);
  }


  Frame::~Frame()
  {
    Terminate();
    this->thread->join();
    frames.erase(this->framenum);
  }
  
  
  // void Frame::Clear(void)
  // {
  //   this->figures.clear();
  //   SendCommand(Frame::Command::Clear;
  //   std::unique_lock<std::mutex> lock(this->mtx);
  //   this->cv.wait(lock);
  // }



////////////////////////////////////////////////////////////////
/// List of all frames
  std::map<int, Frame*> Frame::frames;
  int Frame::lastframenum;


  ////////////////////////////////////////////////////////////////
  /// Communication with render thread

  void Frame::SendCommand(const std::string from, Frame::Command cmd)
  {
    if (!this->thread_alive)
      throw std::runtime_error(from+" : render thread is dead.");

    this->cmd=cmd;
    std::unique_lock<std::mutex> lock(this->mtx);
    this->cv.wait(lock);
  }

  void Frame::Terminate(void)
  {
    SendCommand("Terminate",Frame::Command::Terminate);
  }

  
  ////////////////////////////////////////////////////////////////
  /// Rendering 
  
  class InteractorStyleTrackballCamera : public vtkInteractorStyleTrackballCamera
  {
  public:
    Frame* frame=0;
    
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
        frame->communication_blocked=!frame->communication_blocked;
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

    Frame *frame=0;
    vtkSmartPointer<vtkRenderWindow> window=0;
    vtkSmartPointer<vtkRenderWindowInteractor> interactor=0;

    static TimerCallback *New()    {return new TimerCallback;}
    
    virtual void Execute(
      vtkObject *vtkNotUsed(caller),
      unsigned long eventId,
      void *vtkNotUsed(callData)
      )
    {
      
      if (frame->communication_blocked) return;
      
      if (
        vtkCommand::TimerEvent == eventId  // Check if timer event
        && frame->cmd!=Frame::Command::None  // Check if command has been given
        )
      {
        
        // Lock mutex
        std::unique_lock<std::mutex> lock(frame->mtx);
        
        // Command dispatch
        switch(frame->cmd)
        {
          
          // Add actors to renderer
        case Frame::Command::Show:
        {
          
          for (auto & figure: frame->figures)
          {
            auto &renderer=frame->subframes[figure->framepos].renderer;
            
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
        case Frame::Command::Dump:
        {
          auto imgfilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
          auto pngwriter = vtkSmartPointer<vtkPNGWriter>::New();
          
          pngwriter->SetInputConnection(imgfilter->GetOutputPort());
          pngwriter->SetFileName(frame->fname.c_str());
          
          imgfilter->SetInput(window);
          imgfilter->Update();
          
          interactor->Render();
          pngwriter->Write();
        }
        break;
        
          // Close window and terminate
        case Frame::Command::Resize:
        {
          window->SetSize(frame->win_x, frame->win_y);
        }
        break;
        
        case Frame::Command::Reposition:
        {
          window->SetPosition(frame->win_x, frame->win_y);
        }
        break;
        
          // Close window and terminate
        case Frame::Command::Terminate:
        {
          window->Finalize();
          interactor->TerminateApp();
        }
        break;
        
        default:;
        }
        
        // Clear command
        frame->cmd=Frame::Command::None;
        
        // Notify that command was exeuted
        frame->cv.notify_all();
      }
    }
  };


  
  void Frame::RenderThread(Frame* frame)
  {
    auto window = vtkSmartPointer<vtkRenderWindow>::New();
    auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow(window);
    window->SetSize(frame->win_x, frame->win_y);

    for (auto & subframe : frame->subframes)
    {
      /*
        For Subplots see http://public.kitware.com/pipermail/vtkusers/2009-October/054195.html
        renderer2->SetActiveCamera( renderer1->GetActiveCamera() );
        
      */

      auto renderer = vtkSmartPointer<vtkRenderer>::New();
      subframe.renderer=renderer;
      renderer->SetViewport(subframe.viewport);
      renderer->SetBackground(1., 1., 1.);
      //      renderer->SetUseHiddenLineRemoval(1);
      renderer->GetActiveCamera()->SetPosition(subframe.default_camera_position);
      renderer->GetActiveCamera()->SetFocalPoint(subframe.default_camera_focal_point);
      renderer->GetActiveCamera()->OrthogonalizeViewUp();
      window->AddRenderer(renderer);
    }
    
    interactor->Initialize();

    auto callback =  vtkSmartPointer<TimerCallback>::New();
    callback->interactor=interactor;
    callback->frame=frame;
    callback->window=window;


    auto style =  vtkSmartPointer<InteractorStyleTrackballCamera>::New();
    style->frame=frame;
    interactor->SetInteractorStyle(style);

    interactor->AddObserver(vtkCommand::TimerEvent,callback);
    interactor->Initialize();
    int timerId = interactor->CreateRepeatingTimer(10);
  
    frame->thread_alive=true;
    interactor->Start();
    frame->thread_alive=false;
    frame->cv.notify_all();
    
    interactor->SetRenderWindow(0);
    interactor->TerminateApp();
    window->Finalize();
  }

  void Frame::StartRenderThread(void)
  {
    this->thread=std::make_shared<std::thread>(RenderThread,this);
    do
    {
      std::this_thread::sleep_for (std::chrono::milliseconds(10));
    }
    while (!this->thread_alive);
  }

  void Frame::RestartRenderThread(void)
  {
    this->thread->join();
    StartRenderThread();
  }


  ////////////////////////////////////////////////////////////////
  /// Server communication 

  void Frame::CommunicatorThread(Frame *frame)
  {
    frame->thread_alive=true;
    while(1)
    {
      std::this_thread::sleep_for (std::chrono::milliseconds(5));

      if (frame->cmd!=Frame::Command::None)
      {      

        // Lock mutex
        std::unique_lock<std::mutex> lock(frame->mtx);

        frame->communicator->SendInt(frame->framenum);
        // Command dispatch
        switch(frame->cmd)
        {
          
        case Frame::Command::Show:
        {
          frame->communicator->SendCommand(vtkfig::Command::FrameShow);

          for (auto & figure: frame->figures)
            figure->ServerRTSend(frame->communicator);

        }
        break;
        
        case Frame::Command::Dump:
        {
          frame->communicator->SendCommand(vtkfig::Command::FrameDump);
          frame->communicator->SendString(frame->fname);
        }
        break;
        
        case Frame::Command::Resize:
        {
          frame->communicator->SendCommand(vtkfig::Command::FrameResize);
          frame->communicator->SendInt(frame->win_x);
          frame->communicator->SendInt(frame->win_y);
        }
        break;

        case Frame::Command::Reposition:
        {
          frame->communicator->SendCommand(vtkfig::Command::FrameReposition);
          frame->communicator->SendInt(frame->win_x);
          frame->communicator->SendInt(frame->win_y);
        }
        break;

        case Frame::Command::AddFigure:
        {
          auto figure=frame->figures.back();
          frame->communicator->SendCommand(vtkfig::Command::AddFigure);
          frame->communicator->SendString(figure->SubClassName());
          frame->communicator->SendInt(frame->row(figure->framepos));
          frame->communicator->SendInt(frame->col(figure->framepos));
        }


        case Frame::Command::Clear:
        {
          for (auto & figure: frame->figures)
          {
            
            //frame->communicator->SendCommand(vtkfig::Command::FrameShow);
          }
          
        }
        break;
        
        case Frame::Command::Terminate:
          // Close window and terminate
        {
          frame->communicator->SendCommand(vtkfig::Command::FrameDelete);
        }
        break;
        
        default:
          break;
        }
        
        // Clear command
        frame->cmd=Frame::Command::None;
        
        // Notify that command was exeuted
        frame->cv.notify_all();
      }
    }
  }
  



  void Frame::StartCommunicatorThread(void)
  {
    this->thread=std::make_shared<std::thread>(CommunicatorThread,this);
    do
    {
      std::this_thread::sleep_for (std::chrono::milliseconds(10));
    }
    while (!this->thread_alive);
  }
 
  
}
