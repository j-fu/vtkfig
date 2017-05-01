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
#include "vtkCamera.h"

// Plotxy
#include "vtkPointData.h"
#include "vtkProperty2D.h"
#include "vtkTextProperty.h"
#include "vtkAxisActor2D.h"

// Contour
#include "vtkRectilinearGridGeometryFilter.h"
#include "vtkContourFilter.h"
#include "vtkMarchingContourFilter.h"
#include "vtkOutlineFilter.h"
#include "vtkProperty.h"
#include "vtkLookupTable.h"
#include "vtkColorTransferFunction.h"

// Surf2D
#include "vtkStructuredGridGeometryFilter.h"
#include "vtkWarpScalar.h"
#include "vtkAxesActor.h"
#include "vtkCaptionActor2D.h"


// Quiver
#include "vtkGlyph3D.h"
#include "vtkGlyphSource2D.h"


#include "vtkPolyDataNormals.h"

/*
#include "vtkCaptionActor2D.h"
#include "vtkDataSetMapper.h"
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
#define CMD_BGCOLOR 4
 
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
    bool communication_blocked=false;
    
    /// interactor style
    int interactor_style= INTERACTOR_STYLE_2D;
    
    /// backgroud color
    double bgcolor[3]={1,1,1};
      
    

    Communicator():vtkObjectBase() 
    {
      actors=std::make_shared<std::vector<vtkSmartPointer<vtkProp>>>();
    };

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
          renderer->SetBackground(communicator->bgcolor[0],
                                  communicator->bgcolor[1],
                                  communicator->bgcolor[2]);
          interactor->Render();
        }
        break;

        case CMD_CLEAR:
          // Remove all actors, clear window
        {
          renderer->RemoveAllViewProps();
//          renderer->Clear();
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

    myInteractorStyleImage::SetStyle(interactor,communicator);
    // renderer->GetActiveCamera()->SetPosition(0,0,20);
    // renderer->GetActiveCamera()->SetFocalPoint(0,0,0);
    // renderer->GetActiveCamera()->OrthogonalizeViewUp();
    


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
  
  void Figure::Show()
  {
    if (!communicator->render_thread_alive)
      throw std::runtime_error("Show: render thread is dead");

    communicator->cmd=CMD_SHOW;
    std::unique_lock<std::mutex> lock(communicator->mtx);
    communicator->cv.wait(lock);
  }

  void Figure::Add(Plot &plot)
  {

    for (int i=0;i<plot.actors->size(); i++)
      communicator->actors->push_back(plot.actors->at(i));
    communicator->bgcolor[0]=plot.bgcolor[0];
    communicator->bgcolor[1]=plot.bgcolor[1];
    communicator->bgcolor[2]=plot.bgcolor[2];
  }

  void Figure::Interact()
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
      throw std::runtime_error("Clear: render thread is dead");
    
    communicator->cmd=CMD_CLEAR;
    std::unique_lock<std::mutex> lock(communicator->mtx);
    communicator->cv.wait(lock);
    communicator->actors=std::make_shared<std::vector<vtkSmartPointer<vtkProp>>>();
  }
  
  void Figure::SetInteractorStyle(int istyle)
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
  void Plot::AddActor(vtkSmartPointer<vtkProp> prop) {actors->push_back(prop);}
  bool Plot::IsEmpty(void) {return (actors->size()==0);}
  vtkSmartPointer<vtkLookupTable>  Plot::BuildLookupTable(std::vector<RGBPoint> & xrgb, int size)
  {
    vtkSmartPointer<vtkColorTransferFunction> ctf = vtkSmartPointer<vtkColorTransferFunction>::New();
    for (int i=0;i<xrgb.size(); i++)
      ctf->AddRGBPoint(xrgb[i].x,xrgb[i].r,xrgb[i].g, xrgb[i].b);
    
    vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
    
    lut->SetNumberOfTableValues(size);
    lut->Build();
    
    for(size_t i = 0; i < size; ++i)
     {
       double rgb[3];
       ctf->GetColor(static_cast<double>(i)/(double)size,rgb);
       lut->SetTableValue(i,rgb[0],rgb[1],rgb[2]);
     }
    return lut;
  }

  vtkSmartPointer<vtkScalarBarActor> Plot::BuildColorBar(vtkSmartPointer<vtkPolyDataMapper> mapper)
  {
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
        return colorbar;
  }


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

    Plot::AddActor(xyplot);
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
    RGBTable surface_rgb={{0,0,0,1},{1,1,0,0}};
    RGBTable contour_rgb={{0,0,0,0},{1,0,0,0}};
    surface_lut=BuildLookupTable(surface_rgb,255);
    contour_lut=BuildLookupTable(contour_rgb,2);
  }
  
  
  void Contour2D::Add(vtkSmartPointer<vtkFloatArray> xcoord ,vtkSmartPointer<vtkFloatArray> ycoord ,vtkSmartPointer<vtkFloatArray> values )
  {
    
    if (!Plot::IsEmpty())
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
    double vrange[2];
    gridfunc->GetScalarRange(vrange);


    if (show_surface)
    {
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(geometry->GetOutputPort());
      mapper->SetScalarRange(vrange[0], vrange[1]);
      mapper->SetLookupTable(surface_lut);
      
      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->SetMapper(mapper);
      Plot::AddActor(plot);
      
      if (show_surface_colorbar)
        Plot::AddActor(Plot::BuildColorBar(mapper));
    }


    if (show_contour)
    {
      vtkSmartPointer<vtkContourFilter> contlines = vtkSmartPointer<vtkContourFilter>::New();
      contlines->SetInputConnection(geometry->GetOutputPort());
      double tempdiff = (vrange[1]-vrange[0])/(10*lines);
      contlines->GenerateValues(lines, vrange[0]+tempdiff, vrange[1]-tempdiff);


      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(contlines->GetOutputPort());
      mapper->SetScalarRange(vrange[0], vrange[1]);
      mapper->SetLookupTable(contour_lut);

      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->SetMapper(mapper);
      Plot::AddActor(plot);
      if (show_contour_colorbar)
        Plot::AddActor(Plot::BuildColorBar(mapper));

    }

  }

////////////////////////////////////////////////////////////
  Surf2D::Surf2D(): Plot()
  {
    RGBTable surface_rgb={{0,0,0,1},{1,1,0,0}};
    lut=BuildLookupTable(surface_rgb,255);
  }


  void Surf2D::Add(vtkSmartPointer<vtkStructuredGrid> gridfunc, double Lxy, double Lz)
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

    double vrange[2];
    gridfunc->GetScalarRange(vrange);
    mapper->SetScalarRange(vrange[0], vrange[1]);

    // create plot surface actor
    vtkSmartPointer<vtkActor> surfplot = vtkSmartPointer<vtkActor>::New();
    surfplot->SetMapper(mapper);
    mapper->SetLookupTable(lut);

    // create outline
    vtkSmartPointer<vtkOutlineFilter> outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
    if (do_warp)
	outlinefilter->SetInputConnection(warp->GetOutputPort());
    else
	outlinefilter->SetInputConnection(geometry->GetOutputPort());

    vtkSmartPointer<vtkPolyDataMapper>outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    outlineMapper->SetInputConnection(outlinefilter->GetOutputPort());
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
    vtkSmartPointer<vtkTextProperty> text_prop_ax = axes->GetXAxisCaptionActor2D()->
	GetCaptionTextProperty();
    text_prop_ax->SetColor(0.0, 0.0, 0.0);
    text_prop_ax->SetFontFamilyToArial();
    text_prop_ax->SetFontSize(8);
    axes->GetYAxisCaptionActor2D()->SetCaptionTextProperty(text_prop_ax);
    axes->GetZAxisCaptionActor2D()->SetCaptionTextProperty(text_prop_ax);



    // renderer
    Plot::AddActor(surfplot);
    if (draw_box)
	Plot::AddActor(outline);
    if (draw_axes)
	Plot::AddActor(axes);
    if (show_colorbar)
      Plot::AddActor(Plot::BuildColorBar(mapper));
    
  }



  ////////////////////////////////////////////////////////////
  Quiver2D::Quiver2D(): Plot()  
  {  
    RGBTable quiver_rgb={{0,0,0,1},{1,1,0,0}};
    lut=BuildLookupTable(quiver_rgb,255);
  }
  
  void  Quiver2D::Add(const vtkSmartPointer<vtkFloatArray> xcoord,
                      const vtkSmartPointer<vtkFloatArray> ycoord,
                      const vtkSmartPointer<vtkFloatArray> colors,
                      const vtkSmartPointer<vtkFloatArray> vectors)
  {
    vtkSmartPointer<vtkRectilinearGrid> gridfunc= vtkSmartPointer<vtkRectilinearGrid>::New();
    int Nx = xcoord->GetNumberOfTuples();
    int Ny = ycoord->GetNumberOfTuples();
    gridfunc->SetDimensions(Nx, Ny, 1);
    gridfunc->SetXCoordinates(xcoord);
    gridfunc->SetYCoordinates(ycoord);
    gridfunc->GetPointData()->SetScalars(colors);
    gridfunc->GetPointData()->SetVectors(vectors);

    // filter to geometry primitive
    vtkSmartPointer<vtkRectilinearGridGeometryFilter> geometry =
      vtkSmartPointer<vtkRectilinearGridGeometryFilter>::New();
    geometry->SetInputDataObject(gridfunc);

    // make a vector glyph
    vtkSmartPointer<vtkGlyphSource2D> vec = vtkSmartPointer<vtkGlyphSource2D>::New();
    vec->SetGlyphTypeToArrow();

    vec->SetScale(arrow_scale);
    vec->FilledOff();

    vtkSmartPointer<vtkGlyph3D> glyph = vtkSmartPointer<vtkGlyph3D>::New();
    glyph->SetInputConnection(geometry->GetOutputPort());
    glyph->SetSourceConnection(vec->GetOutputPort());
    glyph->SetColorModeToColorByScalar();

    // map gridfunction
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(glyph->GetOutputPort());
    mapper->SetLookupTable(lut);

    double vrange[2];
    gridfunc->GetScalarRange(vrange);
    mapper->SetScalarRange(vrange[0], vrange[1]);

    // create plot quiver actor
    vtkSmartPointer<vtkActor> quiver_actor = vtkSmartPointer<vtkActor>::New();
    quiver_actor->SetMapper(mapper);



    // create outline
    vtkSmartPointer<vtkOutlineFilter>outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
    outlinefilter->SetInputConnection(geometry->GetOutputPort());
    vtkSmartPointer<vtkPolyDataMapper> outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    outlineMapper->SetInputConnection(outlinefilter->GetOutputPort());
    vtkSmartPointer<vtkActor> outline = vtkSmartPointer<vtkActor>::New();
    outline->SetMapper(outlineMapper);
    outline->GetProperty()->SetColor(0, 0, 0);

    // add actors to renderer
    Plot::AddActor(quiver_actor);
    Plot::AddActor(outline);
    if (show_colorbar)
      Plot::AddActor(Plot::BuildColorBar(mapper));
  }
  


////////////////////////////////////////////////////////////
  Contour3D::Contour3D(): Plot()
  {
    RGBTable slice_rgb={{0,0,0,1},{1,1,0,0}};
    RGBTable contour_rgb={{0,0,0,0},{1,0,0,0}};
    slice_lut=BuildLookupTable(slice_rgb,255);
    contour_lut=BuildLookupTable(contour_rgb,2);
  }
  
  
  void Contour3D::Add(
    vtkSmartPointer<vtkFloatArray> xcoord ,
    vtkSmartPointer<vtkFloatArray> ycoord ,
    vtkSmartPointer<vtkFloatArray> zcoord ,
    vtkSmartPointer<vtkFloatArray> values
    )
  {
    
    if (!Plot::IsEmpty())
      throw std::runtime_error("Contor3D already has data");


    vtkSmartPointer<vtkRectilinearGrid> gridfunc=vtkSmartPointer<vtkRectilinearGrid>::New();

    int Nx = xcoord->GetNumberOfTuples();
    int Ny = ycoord->GetNumberOfTuples();
    int Nz = zcoord->GetNumberOfTuples();

    int nisosurfaces=10;

    // Create rectilinear grid
    gridfunc->SetDimensions(Nx, Ny, Nz);
    gridfunc->SetXCoordinates(xcoord);
    gridfunc->SetYCoordinates(ycoord);
    gridfunc->SetZCoordinates(zcoord);

    gridfunc->GetPointData()->SetScalars(values);


    // filter to geometry primitive
    vtkSmartPointer<vtkRectilinearGridGeometryFilter> geometry =  vtkSmartPointer<vtkRectilinearGridGeometryFilter>::New();
    geometry->SetInputDataObject(gridfunc);

    double vrange[2];
    gridfunc->GetScalarRange(vrange);
    cout << vrange[0] << " " << vrange[1] << endl;

    if (show_slice)
    {
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(geometry->GetOutputPort());
      mapper->SetScalarRange(vrange[0], vrange[1]);
      mapper->SetLookupTable(slice_lut);
      
      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->SetMapper(mapper);
      Plot::AddActor(plot);
      
      if (show_slice_colorbar)
        Plot::AddActor(Plot::BuildColorBar(mapper));
    }


    if (show_contour)
    {
      vtkSmartPointer<vtkContourFilter> isosurfaces = vtkSmartPointer<vtkContourFilter>::New();
      isosurfaces->SetInputConnection(geometry->GetOutputPort());


      double tempdiff = (vrange[1]-vrange[0])/(10*nisosurfaces);
      isosurfaces->GenerateValues(nisosurfaces, vrange[0]+tempdiff, vrange[1]-tempdiff);
      // isosurfaces->SetNumberOfContours(1);
      // isosurfaces->SetValue(0,0.0);
      isosurfaces->ComputeScalarsOn(); 
      isosurfaces->ComputeNormalsOn();
      isosurfaces->Update();



      vtkSmartPointer<vtkPolyDataNormals> isonormals = vtkSmartPointer<vtkPolyDataNormals>::New();
      isonormals->SetInputConnection(isosurfaces->GetOutputPort());
      isonormals->Update();

      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(isosurfaces->GetOutputPort());
      mapper->SetScalarRange(vrange[0], vrange[1]);
      mapper->SetLookupTable(contour_lut);
      mapper->Update();

      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->SetMapper(mapper);
      Plot::AddActor(plot);

      if (show_contour_colorbar)
        Plot::AddActor(Plot::BuildColorBar(mapper));

    }

    // create outline
    vtkSmartPointer<vtkOutlineFilter>outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
    outlinefilter->SetInputConnection(geometry->GetOutputPort());
    vtkSmartPointer<vtkPolyDataMapper> outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    outlineMapper->SetInputConnection(outlinefilter->GetOutputPort());
    vtkSmartPointer<vtkActor> outline = vtkSmartPointer<vtkActor>::New();
    outline->SetMapper(outlineMapper);
    outline->GetProperty()->SetColor(0, 0, 0);
    Plot::AddActor(outline);

  }

  
}



