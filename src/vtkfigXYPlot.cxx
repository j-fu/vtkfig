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

    xyplot = vtkSmartPointer<vtkXYPlotActor>::New();
    iplot=0;
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

  void XYPlot::Build()
  {
    Figure::AddActor2D(xyplot);
  }
  void XYPlot::Clear()
  {
    Figure::ClearActors2D();
    Init();
  }

  void XYPlot::Title(const char * title)
  {
    xyplot->SetTitle(title);
    xyplot->Modified();
  }
  


  void XYPlot::AddPlot(vtkSmartPointer<vtkFloatArray> X,
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


    xyplot->SetPlotColor(iplot, col[0], col[1], col[2]);
    xyplot->SetPlotLines(iplot, plot_lines);
    xyplot->SetPlotPoints(iplot, plot_points);
    xyplot->AddDataSetInput(curve);
    iplot++;
    xyplot->Modified();
  }


}
