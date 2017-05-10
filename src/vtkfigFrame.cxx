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
  ///
  class FrameContent: public vtkObjectBase
  {
  public:
    const int nrow;

    const int ncol;

    FrameContent(const int nrow, const int ncol):
      nrow(nrow),
      ncol(ncol),
      vtkObjectBase() 
    {
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
          cout << irow << " " << icol << " pos: " << ipos << endl;
          cout << row(ipos) << " " << col(ipos) << " pos: " << ipos << endl << endl;

          assert(row(ipos)==irow);
          assert(col(ipos)==icol);
          

          auto & subframe=subframes[ipos];
          subframe.viewport[0]=x;
          subframe.viewport[1]=y-dy;
          subframe.viewport[2]=x+dx;
          subframe.viewport[3]=y;
        }
      }
    };
    static FrameContent *New(int nrow, int ncol)   {      return new FrameContent(nrow, ncol);    }

    FrameContent(const FrameContent& A)=delete;


    enum class Command
    {
      None=0,
        Show,
        Dump,            
        Resize,            
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

    /// window sizes
    int win_x=400;
    int win_y=400;
    
    /// Thread state
    bool render_thread_alive=false;
    
    /// space down state ?
    bool communication_blocked=false;
    
    /// 
    bool wireframe;

    /// 
    struct SubFrame
    {
      SubFrame(){};
      SubFrame(const double vp[4]):viewport{vp[0],vp[1],vp[2],vp[3]}{};
      double default_camera_focal_point[3]={0,0,0};
      double default_camera_position[3]={0,0,20};
      vtkSmartPointer<vtkRenderer>    renderer;
      double viewport[4]={0,0,1,1};
    };

    vtkSmartPointer<vtkfig::Communicator> communicator=0;
    std::shared_ptr<std::thread> render_thread;

    /// Each subframe can hold several figures

    std::vector<std::shared_ptr<Figure>> figures;
    std::vector<SubFrame> subframes;

    
    int pos(const int irow, const int icol) { return irow*ncol+icol;}
    int row(const int pos) { return pos/ncol;}
    int col(const int pos) { return pos%ncol;}
  };

  
  ////////////////////////////////////////////////////////////////
  class InteractorStyleTrackballCamera : public vtkInteractorStyleTrackballCamera
  {
  public:
    vtkSmartPointer<FrameContent> framecontent=0;
    
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
      if(key == "e" || key== "q" || key== "f")
      {
      }
      else if(key == "r")
      {
        for (auto & sf: framecontent->subframes)
        {
          sf.renderer->GetActiveCamera()->SetPosition(sf.default_camera_position);
          sf.renderer->GetActiveCamera()->SetFocalPoint(sf.default_camera_focal_point);
          sf.renderer->GetActiveCamera()->OrthogonalizeViewUp();
          sf.renderer->GetActiveCamera()->SetRoll(0);
        }
      }
      else if(key == "w")
      {
        framecontent->wireframe=!framecontent->wireframe;
        if (framecontent->wireframe)
        {
          for (auto &figure: framecontent->figures)
          {
            for (auto & actor: figure->actors)  actor->GetProperty()->SetRepresentationToWireframe();
//            for (auto actor: figure->actors2d) actor->GetProperty()->SetRepresentationToWireframe();
          }
        }
        else
        {
          for (auto & figure: framecontent->figures)
          {
            for (auto&  actor: figure->actors)  actor->GetProperty()->SetRepresentationToSurface();
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
      vtkSmartPointer<InteractorStyleTrackballCamera> imageStyle = 
        vtkSmartPointer<InteractorStyleTrackballCamera>::New();
      imageStyle->framecontent=framecontent;
      interactor->SetInteractorStyle(imageStyle);
    }
    
  };
  

  ////////////////////////////////////////////////
  class TimerCallback : public vtkCommand
  {
  public:

    vtkSmartPointer<FrameContent> framecontent=0;
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

          for (auto & figure: framecontent->figures)
          {
            auto &renderer=framecontent->subframes[figure->framepos].renderer;
            if (figure->IsEmpty()  || renderer->GetActors()->GetNumberOfItems()==0)
            {
              // This allows clear figure to work
              renderer->RemoveAllViewProps();
              figure->RTBuild();
              for (auto & actor: figure->actors) renderer->AddActor(actor);
              for (auto & actor: figure->actors2d) renderer->AddActor(actor);
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

        case FrameContent::Command::Resize:
          // Close window and terminate
        {
          window->SetSize(framecontent->win_x, framecontent->win_y);
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
    

    vtkSmartPointer<vtkRenderWindow> window = vtkSmartPointer<vtkRenderWindow>::New();
    vtkSmartPointer<vtkRenderWindowInteractor>  interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow(window);
    window->SetSize(framecontent->win_x, framecontent->win_y);

    for (auto & subframe : framecontent->subframes)
    {
      vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
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
    vtkSmartPointer<TimerCallback> callback =  vtkSmartPointer<TimerCallback>::New();


    callback->interactor=interactor;
    callback->framecontent=framecontent;
    callback->window=window;

    InteractorStyleTrackballCamera::SetStyle(interactor,framecontent);


    


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
  Frame::Frame(const int nrow, const int ncol):
    framecontent(FrameContent::New(nrow, ncol))
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
          for (auto figure: framecontent->figures)
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
      

        case FrameContent::Command::Clear:
        {
          for (auto figure: framecontent->figures)
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
  

  Frame::Frame(vtkSmartPointer<vtkfig::Communicator> communicator,const int nrow, const int ncol):
    framecontent(FrameContent::New(nrow,ncol))
  {
    server_mode=true;
    framecontent->communicator=communicator;
    framecontent->communicator->SendCommand(Command::NewFrame);
    framecontent->communicator->SendInt(nrow);
    framecontent->communicator->SendInt(ncol);
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

  void Frame::AddFigure(std::shared_ptr<Figure> fig, int irow, int icol)
  {
    assert(irow<framecontent->nrow);
    assert(icol<framecontent->ncol);

    framecontent->figures.push_back(fig);
    int pos=framecontent->pos(irow,icol);
    fig->framepos=pos;
    if (server_mode)
    {
      framecontent->communicator->SendCommand(Command::AddFigure);
      framecontent->communicator->SendString(fig->SubClassName());
      framecontent->communicator->SendInt(irow);
      framecontent->communicator->SendInt(icol);
      
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

  void Frame::Resize(int x, int y)
  {
    if (!framecontent->render_thread_alive)
      throw std::runtime_error("Resize: render thread is dead");

    framecontent->cmd=FrameContent::Command::Resize;
    framecontent->win_x=x;
    framecontent->win_y=y;
    if (server_mode)
    {
      framecontent->communicator->SendCommand(Command::FrameResize);
      framecontent->communicator->SendInt(x);
      framecontent->communicator->SendInt(y);
    }

    std::unique_lock<std::mutex> lock(framecontent->mtx);
    framecontent->cv.wait(lock);
  }


  
  void Frame::Terminate(void)
  {
    framecontent->cmd=FrameContent::Command::Terminate;
    std::unique_lock<std::mutex> lock(framecontent->mtx);
    framecontent->cv.wait(lock);
  }
  
  // void Frame::Clear(void)
  // {
  //   framecontent->figures.clear();
  //   framecontent->cmd=FrameContent::Command::Clear;
  //   std::unique_lock<std::mutex> lock(framecontent->mtx);
  //   framecontent->cv.wait(lock);
  // }
  
}
