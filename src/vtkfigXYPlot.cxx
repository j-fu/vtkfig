#include "vtkProperty2D.h"
#include "vtkTextProperty.h"
#include "vtkAxisActor2D.h"
#include "vtkLegendBoxActor.h"

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

  void XYPlot::RTBuildVTKPipeline()
  {
//    widget->SetInteractor(interactor);
    xyplot->GetProperty()->SetColor(0.0, 0.0, 0.0);
    xyplot->SetBorder(20);
    xyplot->GetPositionCoordinate()->SetValue(0.0, 0.0, 0);
    xyplot->GetPosition2Coordinate()->SetValue(1.0, 1.0, 0);
    
    xyplot->GetProperty()->SetLineWidth(2);
    xyplot->GetProperty()->SetPointSize(5);
    
    xyplot->SetAdjustXLabels(1);
    xyplot->SetAdjustYLabels(1);
    xyplot->LegendOn();
    xyplot->SetLegendBox(1);
    xyplot->SetLegendBorder(1);

    
    xyplot->PlotPointsOff();
    xyplot->PlotLinesOff();
    xyplot->PlotCurvePointsOn();
    xyplot->PlotCurveLinesOn();
    
    
    xyplot->SetLabelFormat("%2.1f");
    xyplot->SetTitle(title.c_str());
    xyplot->SetXTitle("x");
    xyplot->SetYTitle("y");
    
    int fs=30;
    xyplot->SetTitleFontSize(fs);
    xyplot->SetTitleShadow(0);
    xyplot->SetTitleItalic(0);
    xyplot->SetTitleBold(1);
    xyplot->SetTitleColor(0,0,0);

    xyplot->SetAxisTitleFontSize(fs);
    xyplot->SetAxisTitleItalic(0);
    xyplot->SetAxisTitleShadow(0);
    xyplot->SetAxisTitleColor(0,0,0);

    xyplot->SetAxisLabelFontSize(fs);
    xyplot->SetAxisLabelShadow(0);
    xyplot->SetAxisLabelItalic(0);
    xyplot->SetAxisLabelColor(0,0,0);
    auto text_prop = xyplot->GetLegendActor()->GetEntryTextProperty();

//    xyplot->SetLegendPosition(0.8,0.8);
    // Font scaling is influenced in text actors...
//    text_prop->SetTextScaleMode(vtkTextActor::TEXT_SCALE_MODE_NONE);
    text_prop->ItalicOff();
    text_prop->SetFontFamilyToArial();
    text_prop->SetFontSize(fs);
    xyplot->GetLegendActor()->SetEntryTextProperty(text_prop);


    xyplot->SetXValuesToValue();



    //widget->SetEnabled(1);

    Figure::RTAddActor2D(xyplot);
  }

  void XYPlot::Clear()
  {
    num_plot=0;
    dynXMin=1.0e100;
    dynXMax=-1.0e100;
    dynYMin=1.0e100;
    dynYMax=-1.0e100;
  }

  

  void  XYPlot::ServerRTSend(vtkSmartPointer<internals::Communicator> communicator) 
  {
    communicator->SendString(title);
    communicator->SendString(xtitle);
    communicator->SendString(ytitle);

    communicator->SendDouble(dynXMin);
    communicator->SendDouble(dynXMax);
    communicator->SendDouble(dynYMin);
    communicator->SendDouble(dynYMax);
   
    communicator->SendDouble(fixXMin);
    communicator->SendDouble(fixXMax);
    communicator->SendDouble(fixYMin);
    communicator->SendDouble(fixYMax);

    communicator->SendInt(nxlabels);
    communicator->SendInt(nylabels);
    

    communicator->SendInt(num_plot);
    communicator->SendInt(all_plot_info.size());
    double *data=(double*)all_plot_info.data();
    int ndata=all_plot_info.size()*sizeof(plot_info)/sizeof(double);
    communicator->SendDoubleBuffer(data,ndata);

   

    for (int i=0;i<num_plot;i++)
    {
      auto plot=get_plot_data(i);
      communicator->Send(plot.X,1,1);
      communicator->Send(plot.Y,1,1);
    }
  };
  
  void  XYPlot::ClientMTReceive(vtkSmartPointer<internals::Communicator> communicator) 
  {
    Clear();
    communicator->ReceiveString(title);
    communicator->ReceiveString(xtitle);
    communicator->ReceiveString(ytitle);

    communicator->ReceiveDouble(dynXMin);
    communicator->ReceiveDouble(dynXMax);
    communicator->ReceiveDouble(dynYMin);
    communicator->ReceiveDouble(dynYMax);
   
    communicator->ReceiveDouble(fixXMin);
    communicator->ReceiveDouble(fixXMax);
    communicator->ReceiveDouble(fixYMin);
    communicator->ReceiveDouble(fixYMax);

    communicator->ReceiveInt(nxlabels);
    communicator->ReceiveInt(nylabels);


    int np;
    int npi;
    communicator->ReceiveInt(np);
    communicator->ReceiveInt(npi);
    std::vector<plot_info> new_plot_info(npi);
    double *data=(double*)new_plot_info.data();
    int ndata=npi*sizeof(plot_info)/sizeof(double);
    communicator->ReceiveDoubleBuffer(data,ndata);


    for (int i=0;i<np;i++)
    {
      auto plot=get_plot_data(i);
      communicator->Receive(plot.X,1,1);
      communicator->Receive(plot.Y,1,1);
      LineColorRGB(new_plot_info[i].line_rgb);
      LineType(new_plot_info[i].line_type);
      Legend(new_plot_info[i].legend);
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
    for (int i=0;i<desclen;i++) s[i]=static_cast<char>(next_plot_info.legend[i]);
    xyplot->GetLegendActor()->SetEntryString(num_plot,s);

    

    all_plot_info.insert(all_plot_info.begin()+num_plot,next_plot_info);



    xyplot->SetPlotColor(num_plot, next_plot_info.line_rgb[0], next_plot_info.line_rgb[1], next_plot_info.line_rgb[2]);
    xyplot->SetPlotLines(num_plot, plot_lines);
    xyplot->SetPlotPoints(num_plot, plot_points);

    if (fixXMin>fixXMax)     
      xyplot->SetXRange(dynXMin,dynXMax);
    else
      xyplot->SetXRange(fixXMin,fixXMax);

    if (fixYMin>fixYMax)     
      xyplot->SetYRange(dynYMin,dynYMax);
    else
      xyplot->SetYRange(fixYMin,fixYMax);

    xyplot->SetNumberOfXLabels(nxlabels);
    xyplot->SetNumberOfYLabels(nylabels);

    xyplot->SetLegendPosition2(0.15,0.05*(num_plot+1));


    num_plot++;
    xyplot->Modified();
  }


}
