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

// Plotxy
#include "vtkPointData.h"
#include "vtkProperty2D.h"
#include "vtkTextProperty.h"

// Contour
#include "vtkRectilinearGridGeometryFilter.h"
#include "vtkPolyDataMapper.h"
#include "vtkContourFilter.h"
#include "vtkOutlineFilter.h"
#include "vtkProperty.h"

/*
#include "vtkAxesActor.h"
#include "vtkCamera.h"
#include "vtkCaptionActor2D.h"
#include "vtkDataSetMapper.h"
#include "vtkGlyph3D.h"
#include "vtkGlyphSource2D.h"
#include "vtkPolyData.h"
#include "vtkStructuredGridGeometryFilter.h"
#include "vtkTubeFilter.h"
#include "vtkWarpScalar.h"
*/

#include "visvtk.h"

namespace visvtk
{
  ////////////////////////////////////////////////
  #define CMD_CLEAR 1
  #define CMD_ADDACTORS 2
  #define CMD_UPDATE 3
  #define CMD_DUMP 4
  #define CMD_TERMINATE 5

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
    std::shared_ptr<std::vector<vtkSmartPointer<vtkProp>>> actors;
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
      if (vtkCommand::TimerEvent == eventId && communicator->cmd)
      {
        
        std::unique_lock<std::mutex> lock(communicator->mtx);
        switch(communicator->cmd)
        {
        case CMD_ADDACTORS:
        {
          actors=communicator->actors;
          int nactors=communicator->actors->size();
          for (int i=0;i<nactors;i++)
            renderer->AddActor(communicator->actors->at(i));
          interactor->Render();
          communicator->actors=0;
        }
        case CMD_UPDATE:
        {
          int nactors=actors->size();
          for (int i=0;i<nactors;i++)
            actors->at(i)->Modified();
          interactor->Render();
          communicator->actors=0;
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
          actors=0;
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
        communicator->cmd=0;
        communicator->cv.notify_all();
      }
    }
    vtkSmartPointer<Communicator> communicator;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkRenderWindow> window;
    vtkSmartPointer<vtkRenderWindowInteractor> interactor;
    std::shared_ptr<std::vector<vtkSmartPointer<vtkProp>>> actors;
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
    int timerId = interactor->CreateRepeatingTimer(10);
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
    communicator->cmd=CMD_ADDACTORS;
    std::unique_lock<std::mutex> lock(communicator->mtx);
    communicator->actors=plot.actors;
    communicator->cv.wait(lock);
  }

  void Figure::Update()
  {
    communicator->cmd=CMD_UPDATE;
    std::unique_lock<std::mutex> lock(communicator->mtx);
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
  XYPlot::XYPlot(): 
    Plot(),
    curves(std::make_shared<std::vector<vtkSmartPointer<vtkRectilinearGrid>>>())
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
    // how to read data
    xyplot->SetXValuesToValue();

    actors->push_back(xyplot);
  }

  void XYPlot::Title(const char * title)
  {
    xyplot->SetTitle(title);
    xyplot->Modified();
  }
  
  void XYPlot::Reset(void)
  {
    int ncurves=curves->size();
    for (int i=0;i<ncurves;i++)
      xyplot->RemoveDataSetInput(curves->at(i));
    curves->clear();
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
    vtkSmartPointer<vtkRectilinearGrid> curve = vtkSmartPointer<vtkRectilinearGrid>::New();
    curve->SetDimensions(N, 1, 1);
    curve->SetXCoordinates(X);
    curve->GetPointData()->SetScalars(Y);
    curves->push_back(curve);
    // attach gridfunction to plot
    xyplot->AddDataSetInput(curve);
    
    int plot_no=curves->size()-1;
    
    // set attributes
    xyplot->SetPlotColor(plot_no, col[0], col[1], col[2]);
    xyplot->SetPlotLines(plot_no, plot_lines);
    xyplot->SetPlotPoints(plot_no, plot_points);

  }

  Contour2D::Contour2D()
  {
    Reset();
  }

  void Contour2D::Reset(void )
  {
    actors->clear();
    outline = vtkSmartPointer<vtkActor>::New();
    surfplot = vtkSmartPointer<vtkActor>::New();
    contours = vtkSmartPointer<vtkActor>::New();
    colorbar = vtkSmartPointer<vtkScalarBarActor>::New();
    
    actors->push_back(outline);
    actors->push_back(surfplot);
    actors->push_back(contours);
    actors->push_back(colorbar);

  }


  void Contour2D::Set(vtkSmartPointer<vtkFloatArray> xcoord ,vtkSmartPointer<vtkFloatArray> ycoord ,vtkSmartPointer<vtkFloatArray> values )
  {


    vtkSmartPointer<vtkRectilinearGrid> gridfunc=vtkSmartPointer<vtkRectilinearGrid>::New();
    int Nx = xcoord->GetNumberOfTuples();
    int Ny = ycoord->GetNumberOfTuples();
    int lines=10;

    // Create rectilinear grid
    gridfunc->SetDimensions(Nx, Ny, 1);
    gridfunc->SetXCoordinates(xcoord);
    gridfunc->SetYCoordinates(ycoord);
    gridfunc->GetPointData()->SetScalars(values);

    // filter to geometry primitive
    vtkSmartPointer<vtkRectilinearGridGeometryFilter> geometry =  vtkSmartPointer<vtkRectilinearGridGeometryFilter>::New();
    geometry->SetInputDataObject(gridfunc);

    // map gridfunction
    vtkSmartPointer<vtkPolyDataMapper>mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(geometry->GetOutputPort());

    double tmp[2];
    gridfunc->GetScalarRange(tmp);
    mapper->SetScalarRange(tmp[0], tmp[1]);

    // create plot surface actor
    surfplot->SetMapper(mapper);

    // create contour lines (10 lines)
    vtkSmartPointer<vtkContourFilter> contlines = vtkSmartPointer<vtkContourFilter>::New();
    contlines->SetInputConnection(geometry->GetOutputPort());
    double tempdiff = (tmp[1]-tmp[0])/(10*lines);
    contlines->GenerateValues(lines, tmp[0]+tempdiff, tmp[1]-tempdiff);
    vtkSmartPointer<vtkPolyDataMapper> contourMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    contourMapper->SetInputConnection(contlines->GetOutputPort());
    contourMapper->SetScalarRange(tmp[0], tmp[1]);
    contours->SetMapper(contourMapper);

    // create outline
    vtkSmartPointer<vtkOutlineFilter>outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
    outlinefilter->SetInputConnection(geometry->GetOutputPort());
    vtkSmartPointer<vtkPolyDataMapper>outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    outlineMapper->SetInputDataObject(outlinefilter->GetOutput());



    outline->SetMapper(outlineMapper);
    outline->GetProperty()->SetColor(0, 0, 0);

    // create colorbar
    colorbar->SetLookupTable(contourMapper->GetLookupTable());

    colorbar->SetWidth(0.085);
    colorbar->SetHeight(0.9);
    colorbar->SetPosition(0.9, 0.1);
    vtkSmartPointer<vtkTextProperty> text_prop_cb = colorbar->GetLabelTextProperty();
    text_prop_cb->SetColor(1.0, 1.0, 1.0);
    colorbar->SetLabelTextProperty(text_prop_cb);
  }
  

}
#endif
