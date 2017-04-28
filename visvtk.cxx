#ifndef VISVTK_H
#define VISVTK_H
#include <thread>
#include <mutex>
#include <condition_variable>


#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkWindowToImageFilter.h"
#include "vtkCommand.h"
#include "vtkPNGWriter.h"

#include "vtkPointData.h"
#include "vtkProperty2D.h"
#include "vtkTextProperty.h"

/*
#include "vtkAxesActor.h"
#include "vtkCamera.h"
#include "vtkCaptionActor2D.h"
#include "vtkContourFilter.h"
#include "vtkDataSetMapper.h"
#include "vtkGlyph3D.h"
#include "vtkGlyphSource2D.h"
#include "vtkOutlineFilter.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRectilinearGridGeometryFilter.h"
#include "vtkStructuredGridGeometryFilter.h"
#include "vtkScalarBarActor.h"
#include "vtkTubeFilter.h"
#include "vtkWarpScalar.h"
*/

#include "visvtk.h"

namespace visvtk
{
  ////////////////////////////////////////////////
  #define CMD_CLEAR 1
  #define CMD_ADDACTOR 2
  #define CMD_DUMP 3
  #define CMD_TERMINATE 4

  class Communicator: public vtkObjectBase
  {
  public:
    static Communicator *New()
    {
      return new Communicator;
    }
    int cmd;
    std::mutex mtx;
    std::condition_variable cv;
    vtkSmartPointer<vtkProp> actor=0;
    std::string fname;
    Communicator(const Communicator& A)=delete;
    Communicator():vtkObjectBase() {};
  };


  ////////////////////////////////////////////////
  class TimerCallback : public vtkCommand
  {
  public:
    static TimerCallback *New()
    {
      return new TimerCallback;
    }
    
    virtual void Execute(vtkObject *vtkNotUsed(caller),
                         unsigned long eventId,
                         void *vtkNotUsed(callData))
    {
      if (vtkCommand::TimerEvent == eventId)
      {
        
        std::unique_lock<std::mutex> lock(communicator->mtx);
        switch(communicator->cmd)
        {
        case CMD_ADDACTOR:
        {
          renderer->AddActor(communicator->actor);
          interactor->Render();
          communicator->actor=0;
        }
        break;
        case CMD_DUMP:
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
        case CMD_CLEAR:
        {
          renderer->RemoveAllViewProps();
          renderer->Clear();
        }
        break;
        case CMD_TERMINATE:
        {
          window->Finalize();
          interactor->TerminateApp();
        }
        break;
        default:;
        }
        communicator->cv.notify_all();
      }
    }
    vtkSmartPointer<Communicator> communicator;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkRenderWindow> window;
    vtkSmartPointer<vtkRenderWindowInteractor> interactor;
  };



  ////////////////////////////////////////////////
  void RenderThread(vtkSmartPointer<Communicator> communicator)
  {
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> window = vtkSmartPointer<vtkRenderWindow>::New();
    vtkSmartPointer<vtkRenderWindowInteractor>  interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    window->AddRenderer(renderer);
    interactor->SetRenderWindow(window);
    renderer->SetBackground(1., 1., 1.);
    window->SetSize(400,400);
    interactor->Initialize();
    vtkSmartPointer<TimerCallback> callback =  vtkSmartPointer<TimerCallback>::New();
    callback->renderer=renderer;
    callback->interactor=interactor;
    callback->communicator=communicator;
    callback->window=window;
    interactor->AddObserver(vtkCommand::TimerEvent,callback);
    interactor->Initialize();
    int timerId = interactor->CreateRepeatingTimer(1);
    interactor->Start();
  }


  ////////////////////////////////////////////////
  Figure::Figure():
    communicator(Communicator::New()),
    render_thread(std::make_shared<std::thread>(RenderThread,communicator))
  {}
  
  Figure::~Figure()
  {
    Terminate();
    render_thread->join();
  }
  
  void Figure::Show(Plot &plot)
  {
    communicator->cmd=CMD_ADDACTOR;
    std::unique_lock<std::mutex> lock(communicator->mtx);
    communicator->actor=plot.GetActor();
    communicator->cv.wait(lock);
  }

  void Figure::Dump(std::string fname)
  {
    communicator->cmd=CMD_DUMP;
    communicator->fname=fname;
    std::unique_lock<std::mutex> lock(communicator->mtx);
    communicator->cv.wait(lock);
  }
  
  void Figure::Terminate(void)
  {
    communicator->cmd=CMD_TERMINATE;
    std::unique_lock<std::mutex> lock(communicator->mtx);
    communicator->cv.wait(lock);
  }
  
  void Figure::Clear(void)
  {
    communicator->cmd=CMD_CLEAR;
    std::unique_lock<std::mutex> lock(communicator->mtx);
    communicator->cv.wait(lock);
  }
 

  ////////////////////////////////////////////////
  XYPlot::XYPlot(): Plot()
  {
    xyplot = vtkSmartPointer<vtkXYPlotActor>::New();
    
    xyplot->GetProperty()->SetColor(0.0, 0.0, 0.0);
    xyplot->SetBorder(10);
    xyplot->GetPositionCoordinate()->SetValue(0.0, 0.0, 0);
    xyplot->GetPosition2Coordinate()->SetValue(1.0, 1.0, 0);
    
    xyplot->GetProperty()->SetLineWidth(1);
    xyplot->GetProperty()->SetPointSize(5);
    
    xyplot->PlotPointsOff();
    xyplot->PlotLinesOff();
    xyplot->PlotCurvePointsOn();
    xyplot->PlotCurveLinesOn();
    
    xyplot->SetXValuesToArcLength();
    
    xyplot->SetLabelFormat("%2.1f");
    xyplot->SetTitle("test");
    xyplot->SetXTitle("x");
    xyplot->SetYTitle("y");
    
    vtkSmartPointer<vtkTextProperty>  text_prop = xyplot->GetTitleTextProperty();
    text_prop->SetColor(0.0, 0.0, 0.0);
    text_prop->SetFontFamilyToArial();
    xyplot->SetAxisTitleTextProperty(text_prop);
    xyplot->SetAxisLabelTextProperty(text_prop);
    xyplot->SetTitleTextProperty(text_prop);
  }
  
  void XYPlot::Reset(void)
  {
    for (int i=0;i<plot_no;i++)
    {
      xyplot->RemoveDataSetInput(curves[plot_no]);
      curves[plot_no]=0;
    }
    plot_no=0;
  }


  void XYPlot::Add(vtkSmartPointer<vtkFloatArray> X,
                   vtkSmartPointer<vtkFloatArray> Y, 
                   const double col[3],
                   const std::string linespec)
  {
    int N = X->GetNumberOfTuples();
    int plot_points = 0;
    int plot_lines = 0;
    
    // determine line style
    if (linespec == "-")
      plot_lines = 1;
    else if (linespec == ".")
      plot_points = 1;
    else if (linespec == ".-" || linespec == "-.")
    {
      plot_points = 1;
      plot_lines = 1;
    }
    
    // Make a VTK Rectlinear grid from arrays
    curves[plot_no] = vtkRectilinearGrid::New();
    curves[plot_no]->SetDimensions(N, 1, 1);
    curves[plot_no]->SetXCoordinates(X);
    curves[plot_no]->GetPointData()->SetScalars(Y);
    
    // attach gridfunction to plot
    xyplot->AddDataSetInput(curves[plot_no]);
    
    // how to read data
    xyplot->SetXValuesToValue();
    
    // set attributes
    xyplot->SetPlotColor(plot_no, col[0], col[1], col[2]);
    xyplot->SetPlotLines(plot_no, plot_lines);
    xyplot->SetPlotPoints(plot_no, plot_points);
    plot_no++;
  }
  

}
#endif
