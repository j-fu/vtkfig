#include "vtkProperty2D.h"
#include "vtkTextProperty.h"
#include "vtkAxisActor2D.h"

#include "vtkfigXYPlot.h"

namespace vtkfig
{
  ////////////////////////////////////////////////
  XYPlot::XYPlot():Figure()
  {
    widget = vtkXYPlotWidget::New();
    xyplot  = widget->GetXYPlotActor();
    LineType("-");
  }

  void XYPlot::RTBuild(vtkSmartPointer<vtkRenderWindow> window,
                       vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                       vtkSmartPointer<vtkRenderer> renderer)
  {
    widget->SetInteractor(interactor);
    xyplot->GetProperty()->SetColor(0.0, 0.0, 0.0);
    xyplot->SetBorder(20);
    xyplot->GetPositionCoordinate()->SetValue(0.0, 0.0, 0);
    xyplot->GetPosition2Coordinate()->SetValue(1.0, 1.0, 0);
    
    xyplot->GetProperty()->SetLineWidth(2);
    xyplot->GetProperty()->SetPointSize(5);
    
    xyplot->SetAdjustXLabels(1);
    xyplot->SetAdjustYLabels(1);

    xyplot->PlotPointsOff();
    xyplot->PlotLinesOff();
    xyplot->PlotCurvePointsOn();
    xyplot->PlotCurveLinesOn();
    
    xyplot->SetXValuesToArcLength();
    
    xyplot->SetLabelFormat("%2.1f");
    xyplot->SetTitle(title.c_str());
    xyplot->SetXTitle("x");
    xyplot->SetYTitle("y");
    
    vtkSmartPointer<vtkTextProperty>  text_prop = xyplot->GetTitleTextProperty();
    text_prop->SetColor(0.0, 0.0, 0.0);
    text_prop->ItalicOff();
    text_prop->SetFontFamilyToArial();
    text_prop->SetFontSize(70);
    xyplot->SetAxisTitleTextProperty(text_prop);
    xyplot->SetAxisLabelTextProperty(text_prop);
    xyplot->SetTitleTextProperty(text_prop);
    xyplot->Modified();
    xyplot->SetXValuesToValue();
    //widget->SetEnabled(1);

    Figure::RTAddActor2D(xyplot);
  }

  void XYPlot::Clear()
  {
    num_plot=0;
    all_plot_range[0]=1.0e100;
    all_plot_range[1]=-1.0e100;
    all_plot_range[2]=1.0e100;
    all_plot_range[3]=-1.0e100;
  }

  void XYPlot::Title(const char * xtitle)
  {
    xyplot->SetTitle(xtitle);
    title=xtitle;
    xyplot->Modified();
  }
  

  void  XYPlot::ServerRTSend(vtkSmartPointer<Communicator> communicator) 
  {
    communicator->SendString(title);
    communicator->SendInt(num_plot);
    communicator->SendInt(all_plot_info.size());
    double *data=(double*)all_plot_info.data();
    int ndata=all_plot_info.size()*sizeof(plot_info)/sizeof(double);
    communicator->SendDoubleBuffer(data,ndata);
    communicator->SendDoubleBuffer(all_plot_range,4);
   

    for (int i=0;i<num_plot;i++)
    {
      auto plot=get_plot_data(i);
      communicator->Send(plot.X,1,1);
      communicator->Send(plot.Y,1,1);
    }
  };
  
  void  XYPlot::ClientMTReceive(vtkSmartPointer<Communicator> communicator) 
  {
    Clear();
    communicator->ReceiveString(title);
    int np;
    int npi;
    communicator->ReceiveInt(np);
    communicator->ReceiveInt(npi);
    std::vector<plot_info> new_plot_info(npi);
    double *data=(double*)new_plot_info.data();
    int ndata=npi*sizeof(plot_info)/sizeof(double);
    communicator->ReceiveDoubleBuffer(data,ndata);
    communicator->ReceiveDoubleBuffer(all_plot_range,4);

    for (int i=0;i<np;i++)
    {
      auto plot=get_plot_data(i);
      communicator->Receive(plot.X,1,1);
      communicator->Receive(plot.Y,1,1);
      LineColorRGB(new_plot_info[i].line_rgb);
      LineType(new_plot_info[i].line_type);
      AddPlot();
    }
  };



  void XYPlot::AddPlot()
  {
    auto plot=get_plot_data(num_plot);
    int N = plot.X->GetNumberOfTuples();
    plot.curve->SetDimensions(N, 1, 1);

    int plot_points = 0;
    int plot_lines = 0;
    char s[desclen];
    for (int i=0;i<desclen;i++) s[i]=static_cast<char>(next_plot_info.line_type[i]);
    std::string linespec(s);
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


    all_plot_info.insert(all_plot_info.begin()+num_plot,next_plot_info);



    xyplot->SetPlotColor(num_plot, next_plot_info.line_rgb[0], next_plot_info.line_rgb[1], next_plot_info.line_rgb[2]);
    xyplot->SetPlotLines(num_plot, plot_lines);
    xyplot->SetPlotPoints(num_plot, plot_points);

    xyplot->SetPlotRange(all_plot_range[0],all_plot_range[2],all_plot_range[1],all_plot_range[3]);
    num_plot++;
    xyplot->Modified();
  }


}
