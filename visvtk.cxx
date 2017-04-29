#include <thread>
#include <mutex>
#include <condition_variable>


#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkWindowToImageFilter.h"
#include "vtkCommand.h"
#include "vtkPNGWriter.h"
#include "vtkInteractorStyleImage.h"
#include "vtkInteractorStyleTrackballCamera.h"

// Plotxy
#include "vtkPointData.h"
#include "vtkProperty2D.h"
#include "vtkTextProperty.h"
#include "vtkAxisActor2D.h"
#include "vtkScalarBarActor.h"

// Contour
#include "vtkRectilinearGridGeometryFilter.h"
#include "vtkPolyDataMapper.h"
#include "vtkContourFilter.h"
#include "vtkOutlineFilter.h"
#include "vtkProperty.h"
#include "vtkLookupTable.h"


// Surf2D
#include "vtkStructuredGridGeometryFilter.h"
#include "vtkWarpScalar.h"
#include "vtkAxesActor.h"
#include "vtkCaptionActor2D.h"

/*
#include "vtkCamera.h"
#include "vtkCaptionActor2D.h"
#include "vtkDataSetMapper.h"
#include "vtkGlyph3D.h"
#include "vtkGlyphSource2D.h"
#include "vtkPolyData.h"
#include "vtkTubeFilter.h"
*/

#include "visvtk.h"

namespace visvtk
{
  ////////////////////////////////////////////////
  /// Communicator class Figure-> RenderThread
#define CMD_NONE 0
#define CMD_CLEAR 1
#define CMD_SHOW 2
#define CMD_DUMP 3
#define CMD_TERMINATE 4
#define CMD_INTERACTOR_STYLE 5
 
  class Communicator: public vtkObjectBase
  {
  public:


    /// Communication command
    int cmd; 

    /// mutex to organize communication
    std::mutex mtx; 

    /// condition variable signalizing finished command
    std::condition_variable cv; 

    /// File name to be passed 
    std::string fname; 
    
    /// Actors to be passed
    std::shared_ptr<std::vector<vtkSmartPointer<vtkProp>>> actors; 

    /// Thread state
    bool render_thread_alive=false;

    /// space down state ?
    bool space_down=false;

    /// interactor style
    bool interactor_style= INTERACTOR_STYLE_2D;


    Communicator():vtkObjectBase() {};

    Communicator(const Communicator& A)=delete;

    /// VTK style static constructor
    static Communicator *New()   {      return new Communicator;    }

  };


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
        communicator->space_down=!communicator->space_down;
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
        communicator->space_down=!communicator->space_down;
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

      if (communicator->space_down) return;

      if (
        vtkCommand::TimerEvent == eventId  // Check if timer event
        && communicator->cmd!= CMD_NONE  // Check if command has been given
        )
      {

        // Lock mutex
        std::unique_lock<std::mutex> lock(communicator->mtx);

        // Command dispatch
        switch(communicator->cmd)
        {

          
        case CMD_SHOW:
          // Add actors to renderer
        {
          int nactors=communicator->actors->size();
          for (int i=0;i<nactors;i++)
            renderer->AddActor(communicator->actors->at(i));
          interactor->Render();
          communicator->actors=0;
        }
        break;

        case CMD_CLEAR:
          // Remove all actors, clear window
        {
          renderer->RemoveAllViewProps();
          renderer->Clear();
        }
        break;

        case CMD_DUMP:
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

        case CMD_INTERACTOR_STYLE:
          // Switch interactor style
        {
          switch(communicator->interactor_style)
          {
          case INTERACTOR_STYLE_2D:
            myInteractorStyleImage::SetStyle(interactor,communicator);
            break;
          case INTERACTOR_STYLE_3D:
            myInteractorStyleTrackballCamera::SetStyle(interactor,communicator);
            break;
          default:
            break;
          }
        }
        break;

        case CMD_TERMINATE:
          // Close window and terminate
        {
          window->Finalize();
          interactor->TerminateApp();
        }
        break;
        
        default:;
        }
        
        // Clear command
        communicator->cmd=CMD_NONE;

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

    myInteractorStyleTrackballCamera::SetStyle(interactor,communicator);

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
  Figure::Figure():
    communicator(Communicator::New())
  {
    Start();
  }

  void Figure::Start(void)
  {
    render_thread=std::make_shared<std::thread>(RenderThread,communicator);
    do
    {
      
      std::this_thread::sleep_for (std::chrono::milliseconds(10));
      cout<< "wait\n";
    }
    while (!communicator->render_thread_alive);

  }
  void Figure::Restart(void)
  {
    render_thread->join();
    Start();
  }

  
  Figure::~Figure()
  {
    Terminate();
    render_thread->join();
  }
  
  void Figure::Show(Plot &plot)
  {
    if (!communicator->render_thread_alive)
      //      Restart();
      throw std::runtime_error("Show: render thread is dead");

      

    if (plot.actors->size()==0)
      throw std::runtime_error("No data in plot");

    communicator->cmd=CMD_SHOW;
    std::unique_lock<std::mutex> lock(communicator->mtx);
    communicator->actors=plot.actors;
    communicator->cv.wait(lock);
  }




  void Figure::Dump(std::string fname)
  {
    if (!communicator->render_thread_alive)
      throw std::runtime_error("Dump: render thread is dead");

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
    if (!communicator->render_thread_alive)
      //Restart();
      throw std::runtime_error("Clear: render thread is dead");

    communicator->cmd=CMD_CLEAR;
    std::unique_lock<std::mutex> lock(communicator->mtx);
    communicator->cv.wait(lock);
  }
  
  void Figure::InteractorStyle(int istyle)
  {
    if (!communicator->render_thread_alive)
      //Restart();
      throw std::runtime_error("InteractorStyle: render thread is dead");

    communicator->interactor_style=istyle;
    communicator->cmd=CMD_INTERACTOR_STYLE;
    std::unique_lock<std::mutex> lock(communicator->mtx);
    communicator->cv.wait(lock);
  }

  
  ////////////////////////////////////////////////
  Plot::Plot(): actors(std::make_shared<std::vector<vtkSmartPointer<vtkProp>>>()) {};

  ////////////////////////////////////////////////
  XYPlot::XYPlot():Plot()
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
    text_prop->ItalicOff();
    text_prop->SetFontFamilyToArial();
    text_prop->SetFontSize(40);
    xyplot->SetAxisTitleTextProperty(text_prop);
    xyplot->SetAxisLabelTextProperty(text_prop);
    xyplot->SetTitleTextProperty(text_prop);
    xyplot->Modified();
    xyplot->SetXValuesToValue();

    Plot::actors->push_back(xyplot);
  }

  void XYPlot::Title(const char * title)
  {
    xyplot->SetTitle(title);
    xyplot->Modified();
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


    xyplot->AddDataSetInput(curve);
    xyplot->SetPlotColor(iplot, col[0], col[1], col[2]);
    xyplot->SetPlotLines(iplot, plot_lines);
    xyplot->SetPlotPoints(iplot, plot_points);
    iplot++;
  }

////////////////////////////////////////////////////////////
  Contour2D::Contour2D(): Plot()
  {
  }


  void Contour2D::Add(vtkSmartPointer<vtkFloatArray> xcoord ,vtkSmartPointer<vtkFloatArray> ycoord ,vtkSmartPointer<vtkFloatArray> values )
  {
    
    if (Plot::actors->size()>0)
      throw std::runtime_error("Contor2D already has data");


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
    vtkSmartPointer<vtkActor>     surfplot = vtkSmartPointer<vtkActor>::New();
    surfplot->SetMapper(mapper);



    // create contour lines (10 lines)
    vtkSmartPointer<vtkContourFilter> contlines = vtkSmartPointer<vtkContourFilter>::New();
    contlines->SetInputConnection(geometry->GetOutputPort());
    double tempdiff = (tmp[1]-tmp[0])/(10*lines);
    contlines->GenerateValues(lines, tmp[0]+tempdiff, tmp[1]-tempdiff);


    vtkSmartPointer<vtkLookupTable>  lut = vtkSmartPointer<vtkLookupTable>::New();
    lut->SetHueRange(0,0);
    lut->SetSaturationRange(0,0);
    lut->SetValueRange(0,0);
    
    vtkSmartPointer<vtkPolyDataMapper> contourMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    contourMapper->SetInputConnection(contlines->GetOutputPort());
    contourMapper->SetScalarRange(tmp[0], tmp[1]);
    contourMapper->SetLookupTable(lut);

    vtkSmartPointer<vtkActor>     contours = vtkSmartPointer<vtkActor>::New();
    contours->SetMapper(contourMapper);
    contours->GetProperty()->SetColor(1.0, 0.8941, 0.7686);


    // vtkSmartPointer<vtkOutlineFilter>outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
    // outlinefilter->SetInputConnection(geometry->GetOutputPort());

    // vtkSmartPointer<vtkPolyDataMapper>outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    // outlineMapper->SetInputDataObject(outlinefilter->GetOutput());

    // vtkSmartPointer<vtkActor>     outline = vtkSmartPointer<vtkActor>::New();
    // outline->SetMapper(outlineMapper);
    // outline->GetProperty()->SetColor(0, 0, 0);


    vtkSmartPointer<vtkScalarBarActor>     colorbar = vtkSmartPointer<vtkScalarBarActor>::New();
    colorbar->SetLookupTable(mapper->GetLookupTable());
    colorbar->SetWidth(0.085);
    colorbar->SetHeight(0.9);
    colorbar->SetNumberOfLabels(10);
    colorbar->SetPosition(0.9, 0.1);

    vtkSmartPointer<vtkTextProperty> text_prop_cb = colorbar->GetLabelTextProperty();
    text_prop_cb->SetColor(0,0,0);
    text_prop_cb->SetFontSize(40);
    colorbar->SetLabelTextProperty(text_prop_cb);

    surfplot->PickableOff();
    surfplot->DragableOff();


//        Plot::actors->push_back(outline);
    Plot::actors->push_back(surfplot);
    Plot::actors->push_back(contours);
    Plot::actors->push_back(colorbar);

  }

////////////////////////////////////////////////////////////
  Surf2D::Surf2D(): Plot()
  {
  }


  void Surf2D::Add(vtkSmartPointer<vtkStructuredGrid> gridfunc)
  {

    bool do_warp=true;
    bool draw_box=true;
    bool draw_axes=true;
    bool draw_colorbar=true;
   
    // filter to geometry primitive
    vtkSmartPointer<vtkStructuredGridGeometryFilter>geometry =
      vtkSmartPointer<vtkStructuredGridGeometryFilter>::New();
    geometry->SetInputDataObject(gridfunc);

    // warp to fit in box
    vtkSmartPointer<vtkWarpScalar> warp = vtkSmartPointer<vtkWarpScalar>::New();
    if (do_warp)
    {
	double scale = Lxy/Lz;
	warp->SetInputConnection(geometry->GetOutputPort());
	warp->XYPlaneOn();
	warp->SetScaleFactor(scale);
    }

    // map gridfunction
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    if (do_warp)
	mapper->SetInputConnection(warp->GetOutputPort());
    else
	mapper->SetInputConnection(geometry->GetOutputPort());

    double tmp[2];
    gridfunc->GetScalarRange(tmp);
    mapper->SetScalarRange(tmp[0], tmp[1]);

    // create plot surface actor
    vtkSmartPointer<vtkActor> surfplot = vtkSmartPointer<vtkActor>::New();
    surfplot->SetMapper(mapper);

    // create outline
    vtkSmartPointer<vtkOutlineFilter> outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
    if (do_warp)
	outlinefilter->SetInputConnection(warp->GetOutputPort());
    else
	outlinefilter->SetInputConnection(geometry->GetOutputPort());

    vtkSmartPointer<vtkPolyDataMapper>outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    outlineMapper->SetInputDataObject(outlinefilter->GetOutput());
    vtkSmartPointer<vtkActor> outline = vtkSmartPointer<vtkActor>::New();
    outline->SetMapper(outlineMapper);
    outline->GetProperty()->SetColor(0, 0, 0);

    // create axes
    vtkSmartPointer<vtkAxesActor> axes = vtkSmartPointer<vtkAxesActor>::New();
    axes->SetShaftTypeToCylinder();
    axes->SetNormalizedShaftLength( 0.85, 0.85, 0.85);
    axes->SetNormalizedTipLength( 0.15, 0.15, 0.15);
    axes->SetCylinderRadius( 0.500 * axes->GetCylinderRadius() );
    axes->SetConeRadius( 1.025 * axes->GetConeRadius() );
    axes->SetSphereRadius( 1.500 * axes->GetSphereRadius() );
    vtkTextProperty* text_prop_ax = axes->GetXAxisCaptionActor2D()->
	GetCaptionTextProperty();
    text_prop_ax->SetColor(0.0, 0.0, 0.0);
    text_prop_ax->SetFontFamilyToArial();
    text_prop_ax->SetFontSize(8);
    axes->GetYAxisCaptionActor2D()->GetCaptionTextProperty()->
	ShallowCopy(text_prop_ax);
    axes->GetZAxisCaptionActor2D()->GetCaptionTextProperty()->
	ShallowCopy(text_prop_ax);

    // create colorbar
    vtkSmartPointer<vtkScalarBarActor> colorbar = vtkSmartPointer<vtkScalarBarActor>::New();
    colorbar->SetLookupTable(mapper->GetLookupTable());
    colorbar->SetWidth(0.085);
    colorbar->SetHeight(0.9);
    colorbar->SetPosition(0.9, 0.1);
    vtkSmartPointer<vtkTextProperty> text_prop_cb = colorbar->GetLabelTextProperty();
    text_prop_cb->SetColor(1.0, 1.0, 1.0);
    colorbar->SetLabelTextProperty(text_prop_cb);

    // renderer
    Plot::actors->push_back(surfplot);
    if (draw_box)
	Plot::actors->push_back(outline);
    if (draw_axes)
	Plot::actors->push_back(axes);
    if (draw_colorbar)
	Plot::actors->push_back(colorbar);
    
  }
  
  
}
