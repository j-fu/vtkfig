#include "vtkPointData.h"
#include "vtkProperty2D.h"
#include "vtkTextProperty.h"
#include "vtkAxisActor2D.h"
#include "vtkRectilinearGrid.h"

#include "vtkfigXYPlot.h"

namespace vtkfig
{
  ////////////////////////////////////////////////
  XYPlot::XYPlot():Figure()
  {
    Init();
  }


  void XYPlot::Init()
  {
    xVal.clear();
    yVal.clear();
    xyplot = vtkSmartPointer<vtkXYPlotActor>::New();
    num_plots=0;


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

  }

  void XYPlot::RTBuild()
  {
    Figure::RTAddActor2D(xyplot);
  }

  void XYPlot::Clear()
  {
    /// This is not nice, but I found no way 
    /// to remove data from xyplot without leaking...
    Figure::ClearActors2D();
    Init();
  }

  void XYPlot::Title(const char * title)
  {
    xyplot->SetTitle(title);
    xyplot->Modified();
  }
  

  void  XYPlot::ServerRTSend(vtkSmartPointer<Communicator> communicator) 
  {
    
    communicator->SendInt(num_plots);
    for (int i=0;i<num_plots;i++)
    {
      communicator->Send(xVal[i],1,1);
      communicator->Send(yVal[i],1,1);
    }
  };
  
  void  XYPlot::ClientMTReceive(vtkSmartPointer<Communicator> communicator) 
  {
    Clear();
    int np;
    communicator->ReceiveInt(np);
    for (int i=0;i<np;i++)
    {
      vtkSmartPointer<vtkFloatArray> xVal= vtkSmartPointer<vtkFloatArray>::New();
      vtkSmartPointer<vtkFloatArray> yVal= vtkSmartPointer<vtkFloatArray>::New();
      communicator->Receive(xVal,1,1);
      communicator->Receive(yVal,1,1);
      AddPlot(xVal,yVal);
    }
  };



  void XYPlot::AddPlot(vtkSmartPointer<vtkFloatArray> X,
                       vtkSmartPointer<vtkFloatArray> Y)
  {
    int N = X->GetNumberOfTuples();
    int plot_points = 0;
    int plot_lines = 0;
    
    std::string linespec(line_type);
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


    xyplot->SetPlotColor(num_plots, line_rgb[0], line_rgb[1], line_rgb[2]);
    xyplot->SetPlotLines(num_plots, plot_lines);
    xyplot->SetPlotPoints(num_plots, plot_points);
    xyplot->AddDataSetInput(curve);
    xVal.push_back(X);
    yVal.push_back(Y);
    num_plots++;
    xyplot->Modified();
  }


}
