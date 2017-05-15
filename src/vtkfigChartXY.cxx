#include "vtkfigChartXY.h"

namespace vtkfig
{
  ////////////////////////////////////////////////
  ChartXY::ChartXY():Figure()
  {
    cout <<
R"(
vtkfigChartXY does not (yet?) work as intended:
- How to handle bounds properly ?
- How to prevent leaking ?
Use vtkfigXYPlot instead...
)";

    chartxy  = vtkSmartPointer<vtkChartXY>::New();
    LineType("-");


  }

  void ChartXY::RTBuild(vtkSmartPointer<vtkRenderWindow> window,
                        vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                        vtkSmartPointer<vtkRenderer> renderer)
  {
    chartxyactor= vtkSmartPointer<vtkContextActor>::New(); 
    chartxyactor->GetScene()->AddItem(chartxy);
    Figure::RTAddContextActor(chartxyactor);
//    chartxy->SetForceAxesToBounds(true);
    chartxy->Modified();
  }

  void ChartXY::Clear()
  {
    num_plot=0;
    all_plot_range[0]=1.0e100;
    all_plot_range[1]=-1.0e100;
    all_plot_range[2]=1.0e100;
    all_plot_range[3]=-1.0e100;
  }

  void ChartXY::Title(const char * xtitle)
  {
    chartxy->SetTitle(xtitle);
    title=xtitle;
    chartxy->Modified();
  }
  

  void  ChartXY::ServerRTSend(vtkSmartPointer<Communicator> communicator) 
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
  
  void  ChartXY::ClientMTReceive(vtkSmartPointer<Communicator> communicator) 
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



  void ChartXY::AddPlot()
  {
    auto plot=get_plot_data(num_plot);
    
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

    plot.X->Modified();
    plot.Y->Modified();
    plot.table->Modified();
    plot.line->Modified();
    chartxy->SetForceAxesToBounds(true);

    
    all_plot_info.insert(all_plot_info.begin()+num_plot,next_plot_info);


    // chartxy->SetPlotColor(num_plot, next_plot_info.line_rgb[0], next_plot_info.line_rgb[1], next_plot_info.line_rgb[2]);
    // chartxy->SetPlotLines(num_plot, plot_lines);
    // chartxy->SetPlotPoints(num_plot, plot_points);

    // chartxy->SetPlotRange(all_plot_range[0],all_plot_range[2],all_plot_range[1],all_plot_range[3]);
    num_plot++;
    chartxy->Modified();
  }


}
