#include <mutex>
#include <thread>
#include <condition_variable>

#include "vtkRenderer.h"
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
  ////////////////////////////////////////////////
  class FrameContent: public vtkObjectBase
  {
  public:
    FrameContent():vtkObjectBase() 
    {
      figures=std::make_shared<std::vector<std::shared_ptr<Figure>>>();
    };
    static FrameContent *New()   {      return new FrameContent;    }

    FrameContent(const FrameContent& A)=delete;


    enum class Command
    {
      None=0,
        Show,
        Dump,            
        Clear,            
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

    std::shared_ptr<std::vector<std::shared_ptr<Figure>>> figures;
    
    /// Thread state
    bool render_thread_alive=false;
    
    /// space down state ?
    bool communication_blocked=false;
    
    /// interactor style
    Frame::InteractorStyle interactor_style= Frame::InteractorStyle::Volumetric;

    /// 
    bool wireframe;

    /// 
    double default_camera_focal_point[3]={0,0,0};
    double default_camera_position[3]={0,0,20};
    
    std::shared_ptr<std::thread> render_thread;

    vtkSmartPointer<vtkfig::Communicator> communicator=0;
    vtkSmartPointer<vtkRenderer> renderer=0;

  };



  ////////////////////////////////////////////////////////////////
  class myInteractorStyleImage : public vtkInteractorStyleImage
  {
  public:
    vtkSmartPointer<FrameContent> framecontent=0;
    
    static myInteractorStyleImage* New()
    {
      return new myInteractorStyleImage();
    }
    myInteractorStyleImage(): vtkInteractorStyleImage() {};
    
    
    virtual void OnChar() 
    {
      // Get the keypress
      vtkRenderWindowInteractor *interactor = this->Interactor;
      std::string key = interactor->GetKeySym();
      
      if(key == "e" || key== "q")
      {
        std::cout << "Exit keys are disabled" << std::endl;
      }
      else if (key=="space")
      {
        framecontent->communication_blocked=!framecontent->communication_blocked;
      }
      else
        vtkInteractorStyleImage::OnChar();
    }
    
    static void SetStyle(vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                         vtkSmartPointer<FrameContent> framecontent)
    {
      vtkSmartPointer<myInteractorStyleImage> imageStyle = 
        vtkSmartPointer<myInteractorStyleImage>::New();
      imageStyle->framecontent=framecontent;
      interactor->SetInteractorStyle(imageStyle);
    }
    
  };
  
  ////////////////////////////////////////////////////////////////
  class myInteractorStyleTrackballCamera : public vtkInteractorStyleTrackballCamera
  {
  public:
    vtkSmartPointer<FrameContent> framecontent=0;
    
    static myInteractorStyleTrackballCamera* New()
    {
      return new myInteractorStyleTrackballCamera();
    }
    myInteractorStyleTrackballCamera(): vtkInteractorStyleTrackballCamera() {};
    
    
    virtual void OnChar() 
    {
      // Get the keypress
      vtkRenderWindowInteractor *interactor = this->Interactor;
      std::string key = interactor->GetKeySym();
      if(key == "e" || key== "q" || key== "f")
      {
      }
      else if(key == "r")
      {
        framecontent->renderer->GetActiveCamera()->SetPosition(framecontent->default_camera_position);
        framecontent->renderer->GetActiveCamera()->SetFocalPoint(framecontent->default_camera_focal_point);
        framecontent->renderer->GetActiveCamera()->OrthogonalizeViewUp();
        framecontent->renderer->GetActiveCamera()->SetRoll(0);
      }
      else if(key == "w")
      {
        framecontent->wireframe=!framecontent->wireframe;
        if (framecontent->wireframe)
        {
          for (auto figure: *framecontent->figures)
          {
            for (auto actor: figure->actors)  actor->GetProperty()->SetRepresentationToWireframe();
//            for (auto actor: figure->actors2d) actor->GetProperty()->SetRepresentationToWireframe();
          }
        }
        else
        {
          for (auto figure: *framecontent->figures)
          {
            for (auto actor: figure->actors)  actor->GetProperty()->SetRepresentationToSurface();
//            for (auto actor: figure->actors2d) actor->GetProperty()->SetRepresentationToSurface();
          }
        }
      }
      else if (key=="space")
      {
        framecontent->communication_blocked=!framecontent->communication_blocked;
      }
      else if(key == "h" or key == "?")
      {
        cout << 
R"(
Key     Action

space  Interrupt/continue calculation
r      Reset camera
w      Wireframe modus
)";
      }
      else
        vtkInteractorStyleTrackballCamera::OnChar();
    }
    
    static void SetStyle(vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                         vtkSmartPointer<FrameContent> framecontent)
    {
      vtkSmartPointer<myInteractorStyleTrackballCamera> imageStyle = 
        vtkSmartPointer<myInteractorStyleTrackballCamera>::New();
      imageStyle->framecontent=framecontent;
      interactor->SetInteractorStyle(imageStyle);
    }
    
  };
  

  ////////////////////////////////////////////////
  class TimerCallback : public vtkCommand
  {
  public:

    vtkSmartPointer<FrameContent> framecontent=0;
    vtkSmartPointer<vtkRenderer> renderer=0;
    vtkSmartPointer<vtkRenderWindow> window=0;
    vtkSmartPointer<vtkRenderWindowInteractor> interactor=0;

    static TimerCallback *New()    {      return new TimerCallback;    }
    
    virtual void Execute(vtkObject *vtkNotUsed(caller),
                         unsigned long eventId,
                         void *vtkNotUsed(callData))
    {

      if (framecontent->communication_blocked) return;

      if (
        vtkCommand::TimerEvent == eventId  // Check if timer event
        && framecontent->cmd!=FrameContent::Command::None  // Check if command has been given
        )
      {

        // Lock mutex
        std::unique_lock<std::mutex> lock(framecontent->mtx);

        // Command dispatch
        switch(framecontent->cmd)
        {

          
        case FrameContent::Command::Show:
          // Add actors to renderer
        {

          for (auto figure: *framecontent->figures)
          {
            if (figure->IsEmpty()  || renderer->GetActors()->GetNumberOfItems()==0)
            {
              // This allows clear figure to work
              renderer->RemoveAllViewProps();
              figure->RTBuild();
              for (auto actor: figure->actors) renderer->AddActor(actor);
              for (auto actor: figure->actors2d) renderer->AddActor(actor);
              figure->RTSetInteractor(interactor);
            }
            figure->RTUpdateActors();
            renderer->SetBackground(figure->bgcolor[0],
                                    figure->bgcolor[1],
                                    figure->bgcolor[2]);
          }
          interactor->Render();
        }
        break;

        case FrameContent::Command::Dump:
          // Write picture to file
        {
          vtkSmartPointer<vtkWindowToImageFilter> imgfilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
          vtkSmartPointer<vtkPNGWriter> pngwriter = vtkSmartPointer<vtkPNGWriter>::New();
          
          pngwriter->SetInputConnection(imgfilter->GetOutputPort());
          pngwriter->SetFileName(framecontent->fname.c_str());
          
          imgfilter->SetInput(window);
          imgfilter->Update();
          
          interactor->Render();
          pngwriter->Write();
        }
        break;

        case FrameContent::Command::SetInteractorStyle:
          // Switch interactor style
        {
          switch(framecontent->interactor_style)
          {
          case Frame::InteractorStyle::Planar:
            myInteractorStyleImage::SetStyle(interactor,framecontent);
            break;
          case  Frame::InteractorStyle::Volumetric:
            myInteractorStyleTrackballCamera::SetStyle(interactor,framecontent);
            break;
          default:
            break;
          }
        }
        break;


        case FrameContent::Command::Clear:
          // Close window and terminate
        {
          renderer->RemoveAllViewProps();
        }
        break;

        case FrameContent::Command::Terminate:
          // Close window and terminate
        {
          window->Finalize();
          interactor->TerminateApp();
        }
        break;
        
        default:;
        }
        
        // Clear command
        framecontent->cmd=FrameContent::Command::None;

        // Notify that command was exeuted
        framecontent->cv.notify_all();
      }
    }
  };

  /*
    For Subplots see http://public.kitware.com/pipermail/vtkusers/2009-October/054195.html

   */
  ////////////////////////////////////////////////
  void RenderThread(vtkSmartPointer<FrameContent> framecontent)
  {
    

    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> window = vtkSmartPointer<vtkRenderWindow>::New();
    vtkSmartPointer<vtkRenderWindowInteractor>  interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    window->AddRenderer(renderer);
    interactor->SetRenderWindow(window);
//    renderer->SetViewport(0.0,0.0,0.5,0.5);
    renderer->SetBackground(1., 1., 1.);

    window->SetSize(400,400);
    interactor->Initialize();
    vtkSmartPointer<TimerCallback> callback =  vtkSmartPointer<TimerCallback>::New();

    callback->renderer=renderer;
    callback->interactor=interactor;
    callback->framecontent=framecontent;
    callback->window=window;

    myInteractorStyleTrackballCamera::SetStyle(interactor,framecontent);

    framecontent->renderer=renderer; 
    renderer->GetActiveCamera()->SetPosition(framecontent->default_camera_position);
    renderer->GetActiveCamera()->SetFocalPoint(framecontent->default_camera_focal_point);
    renderer->GetActiveCamera()->OrthogonalizeViewUp();
    


    interactor->AddObserver(vtkCommand::TimerEvent,callback);
    interactor->Initialize();
    int timerId = interactor->CreateRepeatingTimer(10);
  
    framecontent->render_thread_alive=true;
    interactor->Start();
    framecontent->render_thread_alive=false;
    framecontent->cv.notify_all();
    
    interactor->SetRenderWindow(0);
    interactor->TerminateApp();
    window->Finalize();
  }





  ////////////////////////////////////////////////
  Frame::Frame():
    framecontent(FrameContent::New())
  {
    server_mode=false;
    Start();
  }



  void CommunicatorThread(vtkSmartPointer<FrameContent> framecontent)
  {
    framecontent->render_thread_alive=true;
    while(1)
    {
      std::this_thread::sleep_for (std::chrono::milliseconds(5));

      if (framecontent->cmd!=FrameContent::Command::None)
      {      
        // Lock mutex
        std::unique_lock<std::mutex> lock(framecontent->mtx);
        // Command dispatch
        switch(framecontent->cmd)
        {
                    
        case FrameContent::Command::Show:
        {
          for (auto figure: *framecontent->figures)
          {
            framecontent->communicator->SendCommand(vtkfig::Command::FrameShow);
            figure->ServerRTSend(framecontent->communicator);
          }
        }
        break;
        
        case FrameContent::Command::Dump:
          // Write picture to file
        {
        }
        break;
      
        case FrameContent::Command::SetInteractorStyle:
          // Switch interactor style
        {
          framecontent->communicator->SendCommand(vtkfig::Command::SetInteractorStyle);
          int istyle=static_cast<int>(framecontent->interactor_style);
          framecontent->communicator->SendInt(istyle);
        }
        break;
      
      
        case FrameContent::Command::Clear:
        {
          for (auto figure: *framecontent->figures)
          {

            //framecontent->communicator->SendCommand(vtkfig::Command::FrameShow);
          }
          
        }
        break;
        
        case FrameContent::Command::Terminate:
          // Close window and terminate
        {
        }
        break;
        
        default:
          break;
        }
      
        // Clear command
        framecontent->cmd=FrameContent::Command::None;
      
        // Notify that command was exeuted
        framecontent->cv.notify_all();
      }
    }
  }
  

  Frame::Frame(vtkSmartPointer<vtkfig::Communicator> communicator):
    framecontent(FrameContent::New())
  {
    server_mode=true;
    framecontent->communicator=communicator;
    framecontent->communicator->SendCommand(Command::NewFrame);
    framecontent->render_thread=std::make_shared<std::thread>(CommunicatorThread,framecontent);
    do
    {
      std::this_thread::sleep_for (std::chrono::milliseconds(10));
    }
    while (!framecontent->render_thread_alive);
  }


  


  void Frame::Start(void)
  {
    framecontent->render_thread=std::make_shared<std::thread>(RenderThread,framecontent);
    do
    {
      std::this_thread::sleep_for (std::chrono::milliseconds(10));
    }
    while (!framecontent->render_thread_alive);

  }
  void Frame::Restart(void)
  {
    framecontent->render_thread->join();
    Start();
  }

  
  Frame::~Frame()
  {
    Terminate();
    framecontent->render_thread->join();
  }
  
  void Frame::Show()
  {
    if (!framecontent->render_thread_alive)
      throw std::runtime_error("Show: render thread is dead");

    framecontent->cmd=FrameContent::Command::Show;
    std::unique_lock<std::mutex> lock(framecontent->mtx);
    framecontent->cv.wait(lock);
  }

  void Frame::AddFigure(std::shared_ptr<Figure> fig)
  {
    framecontent->figures->push_back(fig);
    if (server_mode)
    {
      framecontent->communicator->SendCommand(Command::AddFigure);
      framecontent->communicator->SendString(fig->SubClassName());
    }
  }


  void Frame::Interact()
  {
    if (!framecontent->render_thread_alive)
      throw std::runtime_error("Show: render thread is dead");
    framecontent->communication_blocked=true;
    do
    {
      std::this_thread::sleep_for (std::chrono::milliseconds(10));
    }
    while (framecontent->communication_blocked);

  }




  void Frame::Dump(std::string fname)
  {
    if (!framecontent->render_thread_alive)
      throw std::runtime_error("Dump: render thread is dead");

    framecontent->cmd=FrameContent::Command::Dump;
    framecontent->fname=fname;
    std::unique_lock<std::mutex> lock(framecontent->mtx);
    framecontent->cv.wait(lock);
  }
  
  void Frame::Terminate(void)
  {
    framecontent->cmd=FrameContent::Command::Terminate;
    std::unique_lock<std::mutex> lock(framecontent->mtx);
    framecontent->cv.wait(lock);
  }
  
  void Frame::Clear(void)
  {
    framecontent->figures=std::make_shared<std::vector<std::shared_ptr<Figure>>>();
    framecontent->cmd=FrameContent::Command::Clear;
    std::unique_lock<std::mutex> lock(framecontent->mtx);
    framecontent->cv.wait(lock);
  }
  
  void Frame::SetInteractorStyle(Frame::InteractorStyle istyle)
  {
    if (!framecontent->render_thread_alive)
      //Restart();
      throw std::runtime_error("InteractorStyle: render thread is dead");

    framecontent->interactor_style=istyle;
    framecontent->cmd=FrameContent::Command::SetInteractorStyle;
    std::unique_lock<std::mutex> lock(framecontent->mtx);
    framecontent->cv.wait(lock);
  }

}
