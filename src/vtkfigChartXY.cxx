#include <vtkAxis.h>
#include <vtkContextView.h>
#include <vtkContextScene.h>

#include "vtkfigChartXY.h"
#include "config.h"

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
    chartxy->SetInteractive(true);
    LineType("-");


  }


  void ChartXY::RTBuildAllVTKPipelines(vtkSmartPointer<vtkRenderer> renderer) 
  {
    ctxactor= vtkSmartPointer<vtkContextActor>::New();
    ctxactor->RenderOverlay(renderer);
 
    ctxactor->GetScene()->AddItem(chartxy);

    Figure::RTAddContextActor(ctxactor);
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
  

  void  ChartXY::ServerRTSend(vtkSmartPointer<internals::Communicator> communicator) 
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
      auto line=chartxy->GetPlot(i);
      auto X=vtkDoubleArray::SafeDownCast(line->GetInput()->GetColumn(0));
      auto Y=vtkDoubleArray::SafeDownCast(line->GetInput()->GetColumn(1));
      communicator->Send(X,1,1);
      communicator->Send(Y,1,1);
    }
  };
  
  void  ChartXY::ClientMTReceive(vtkSmartPointer<internals::Communicator> communicator) 
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
     
      auto X=vtkSmartPointer<vtkDoubleArray>::New();
      auto Y=vtkSmartPointer<vtkDoubleArray>::New();

      communicator->Receive(X,1,1);
      communicator->Receive(Y,1,1);
      LineColorRGB(new_plot_info[i].line_rgb);
      LineType(new_plot_info[i].line_type);
      AddPlot();
    }
  };



  void ChartXY::AddPlot()
  {
    

    // chartxy->GetAxis(0)->SetRange(all_plot_range[0],all_plot_range[1]);
    // chartxy->GetAxis(1)->SetRange(all_plot_range[2],all_plot_range[3]);


    // chartxy->SetPlotColor(num_plot, next_plot_info.line_rgb[0], next_plot_info.line_rgb[1], next_plot_info.line_rgb[2]);
    // chartxy->SetPlotLines(num_plot, plot_lines);
    // chartxy->SetPlotPoints(num_plot, plot_points);

//    chartxy->SetPlotRange(all_plot_range[0],all_plot_range[2],all_plot_range[1],all_plot_range[3]);
    chartxy->RecalculateBounds();
    num_plot++;
    //    chartxy->RecalculateBounds();
    chartxy->Modified();
  }


}
