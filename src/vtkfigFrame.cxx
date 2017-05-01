#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleImage.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkWindowToImageFilter.h"
#include "vtkPNGWriter.h"



#include "vtkfigFrame.h"
#include "vtkfigCommunicator.h"
#include "vtkfigFigure.h"

namespace vtkfig
{

  ////////////////////////////////////////////////////////////////
  class myInteractorStyleImage : public vtkInteractorStyleImage
  {
  public:
    vtkSmartPointer<Communicator> communicator=0;
    
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
        communicator->communication_blocked=!communicator->communication_blocked;
      }
      else
        vtkInteractorStyleImage::OnChar();
    }
    
    static void SetStyle(vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                         vtkSmartPointer<Communicator> communicator)
    {
      vtkSmartPointer<myInteractorStyleImage> imageStyle = 
        vtkSmartPointer<myInteractorStyleImage>::New();
      imageStyle->communicator=communicator;
      interactor->SetInteractorStyle(imageStyle);
    }
    
  };
  
  ////////////////////////////////////////////////////////////////
  class myInteractorStyleTrackballCamera : public vtkInteractorStyleTrackballCamera
  {
  public:
    vtkSmartPointer<Communicator> communicator=0;
    
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
      
      if(key == "e" || key== "q")
      {
        std::cout << "Exit keys are disabled" << std::endl;
      }
      else if (key=="space")
      {
        communicator->communication_blocked=!communicator->communication_blocked;
      }
      else
        vtkInteractorStyleTrackballCamera::OnChar();
    }
    
    static void SetStyle(vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                         vtkSmartPointer<Communicator> communicator)
    {
      vtkSmartPointer<myInteractorStyleTrackballCamera> imageStyle = 
        vtkSmartPointer<myInteractorStyleTrackballCamera>::New();
      imageStyle->communicator=communicator;
      interactor->SetInteractorStyle(imageStyle);
    }
    
  };
  

  ////////////////////////////////////////////////
  class TimerCallback : public vtkCommand
  {
  public:

    vtkSmartPointer<Communicator> communicator=0;
    vtkSmartPointer<vtkRenderer> renderer=0;
    vtkSmartPointer<vtkRenderWindow> window=0;
    vtkSmartPointer<vtkRenderWindowInteractor> interactor=0;

    static TimerCallback *New()    {      return new TimerCallback;    }
    
    virtual void Execute(vtkObject *vtkNotUsed(caller),
                         unsigned long eventId,
                         void *vtkNotUsed(callData))
    {

      if (communicator->communication_blocked) return;

      if (
        vtkCommand::TimerEvent == eventId  // Check if timer event
        && communicator->cmd!=Communicator::Command::None  // Check if command has been given
        )
      {

        // Lock mutex
        std::unique_lock<std::mutex> lock(communicator->mtx);

        // Command dispatch
        switch(communicator->cmd)
        {

          
        case Communicator::Command::Show:
          // Add actors to renderer
        {
          renderer->RemoveAllViewProps();
          int nactors=communicator->actors->size();
          for (int i=0;i<nactors;i++)
            renderer->AddActor(communicator->actors->at(i));
          renderer->SetBackground(communicator->bgcolor[0],
                                  communicator->bgcolor[1],
                                  communicator->bgcolor[2]);
          interactor->Render();
        }
        break;

        case Communicator::Command::Dump:
          // Write picture to file
        {
          vtkSmartPointer<vtkWindowToImageFilter> imgfilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
          vtkSmartPointer<vtkPNGWriter> pngwriter = vtkSmartPointer<vtkPNGWriter>::New();
          
          pngwriter->SetInputConnection(imgfilter->GetOutputPort());
          pngwriter->SetFileName(communicator->fname.c_str());
          
          imgfilter->SetInput(window);
          imgfilter->Update();
          
          interactor->Render();
          pngwriter->Write();
        }
        break;

        case Communicator::Command::SetInteractorStyle:
          // Switch interactor style
        {
          switch(communicator->interactor_style)
          {
          case Frame::InteractorStyle::Planar:
            myInteractorStyleImage::SetStyle(interactor,communicator);
            break;
          case  Frame::InteractorStyle::Volumetric:
            myInteractorStyleTrackballCamera::SetStyle(interactor,communicator);
            break;
          default:
            break;
          }
        }
        break;

        case Communicator::Command::Terminate:
          // Close window and terminate
        {
          window->Finalize();
          interactor->TerminateApp();
        }
        break;
        
        default:;
        }
        
        // Clear command
        communicator->cmd=Communicator::Command::None;

        // Notify that command was exeuted
        communicator->cv.notify_all();
      }
    }
  };

  /*
    For Subplots see http://public.kitware.com/pipermail/vtkusers/2009-October/054195.html

   */
  ////////////////////////////////////////////////

  void RenderThread(vtkSmartPointer<Communicator> communicator)
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
    callback->communicator=communicator;
    callback->window=window;

    myInteractorStyleImage::SetStyle(interactor,communicator);
    renderer->GetActiveCamera()->SetPosition(0,0,20);
    renderer->GetActiveCamera()->SetFocalPoint(0,0,0);
    renderer->GetActiveCamera()->OrthogonalizeViewUp();
    


    interactor->AddObserver(vtkCommand::TimerEvent,callback);
    interactor->Initialize();
    int timerId = interactor->CreateRepeatingTimer(10);
  
    communicator->render_thread_alive=true;
    interactor->Start();
    communicator->render_thread_alive=false;
    communicator->cv.notify_all();
    
    interactor->SetRenderWindow(0);
    interactor->TerminateApp();
    window->Finalize();
  }



  ////////////////////////////////////////////////
  Frame::Frame():
    communicator(Communicator::New())
  {
    Start();
  }

  void Frame::Start(void)
  {
    render_thread=std::make_shared<std::thread>(RenderThread,communicator);
    do
    {
      std::this_thread::sleep_for (std::chrono::milliseconds(10));
    }
    while (!communicator->render_thread_alive);

  }
  void Frame::Restart(void)
  {
    render_thread->join();
    Start();
  }

  
  Frame::~Frame()
  {
    Terminate();
    render_thread->join();
  }
  
  void Frame::Show()
  {
    if (!communicator->render_thread_alive)
      throw std::runtime_error("Show: render thread is dead");

    communicator->cmd=Communicator::Command::Show;
    std::unique_lock<std::mutex> lock(communicator->mtx);
    communicator->cv.wait(lock);
  }

  void Frame::Add(Figure &plot)
  {

    for (int i=0;i<plot.actors->size(); i++)
      communicator->actors->push_back(plot.actors->at(i));
    communicator->bgcolor[0]=plot.bgcolor[0];
    communicator->bgcolor[1]=plot.bgcolor[1];
    communicator->bgcolor[2]=plot.bgcolor[2];
  }

  void Frame::Interact()
  {
    if (!communicator->render_thread_alive)
      throw std::runtime_error("Show: render thread is dead");
    communicator->communication_blocked=true;
    do
    {
      std::this_thread::sleep_for (std::chrono::milliseconds(10));
    }
    while (communicator->communication_blocked);

  }




  void Frame::Dump(std::string fname)
  {
    if (!communicator->render_thread_alive)
      throw std::runtime_error("Dump: render thread is dead");

    communicator->cmd=Communicator::Command::Dump;
    communicator->fname=fname;
    std::unique_lock<std::mutex> lock(communicator->mtx);
    communicator->cv.wait(lock);
  }
  
  void Frame::Terminate(void)
  {
    communicator->cmd=Communicator::Command::Terminate;
    std::unique_lock<std::mutex> lock(communicator->mtx);
    communicator->cv.wait(lock);
  }
  
  void Frame::Clear(void)
  {

    // if (!communicator->render_thread_alive)
    //   throw std::runtime_error("Clear: render thread is dead");
    
    // communicator->cmd=Communicator::Command::CLEAR;
    // std::unique_lock<std::mutex> lock(communicator->mtx);
    // communicator->cv.wait(lock);
    communicator->actors=std::make_shared<std::vector<vtkSmartPointer<vtkProp>>>();
  }
  
  void Frame::SetInteractorStyle(Frame::InteractorStyle istyle)
  {
    if (!communicator->render_thread_alive)
      //Restart();
      throw std::runtime_error("InteractorStyle: render thread is dead");

    communicator->interactor_style=istyle;
    communicator->cmd=Communicator::Command::SetInteractorStyle;
    std::unique_lock<std::mutex> lock(communicator->mtx);
    communicator->cv.wait(lock);
  }

}
