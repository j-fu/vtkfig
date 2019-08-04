/**
    \file vtkfigChartXY.h

    Experimental class trying to use the chartxy widget - don't use!
*/

#ifndef VTKFIG_CHARTXY_H
#define VTKFIG_CHARTXY_H

#include <vtkDoubleArray.h>
#include <vtkChartXY.h>
#include <vtkTable.h>
#include <vtkPlot.h>
#include <vtkPointData.h>
#include <vtkContextActor.h>


#include "vtkfigFigure.h"

namespace vtkfig
{
  ///
  /// Experimental class trying to use the chartxy widget - don't use!
  /// 
  class ChartXY: public Figure
  {
    
  public:
    ChartXY();
    static std::shared_ptr<ChartXY> New() { return std::make_shared<ChartXY>(); }

    std::string SubClassName() override final {return std::string("ChartXY");}

    void Title(const char *title);

    template <typename T> 
    void LineType(const T *type) 
    { for (int i=0;i<desclen;i++) 
      {
        next_plot_info.line_type[i]=static_cast<double>(type[i]);
        if (type[i]=='\0') break;
      }
    }
    void LineColorRGB(double r, double g, double b) { next_plot_info.line_rgb[0]=r; next_plot_info.line_rgb[1]=g; next_plot_info.line_rgb[2]=b;}

    void LineColorRGB(double rgb[3]) { next_plot_info.line_rgb[0]=rgb[0]; next_plot_info.line_rgb[1]=rgb[1]; next_plot_info.line_rgb[2]=rgb[2];}
    
    template<typename V>  void AddPlot(const V &x,  const V &y);
    
    void Clear();



  private:
    

    int num_plot=0;
    void ServerMPSend(vtkSmartPointer<internals::Communicator> communicator) override final;

    void ClientMPReceive(vtkSmartPointer<internals::Communicator> communicator) override final; 


    void AddPlot();

    virtual void RTBuildAllVTKPipelines(vtkSmartPointer<vtkRenderer> renderer) override final; 


    
    vtkSmartPointer<vtkChartXY> chartxy;
    vtkSmartPointer<vtkContextActor> ctxactor; 
    static const int desclen=4;
    std::string title="test";
    

    struct plot_info
    {
      double line_type[desclen]={'-',0,0,0};
      double line_rgb[3]={0,0,0};
      plot_info(){};
      ~plot_info(){};
    };
    
    plot_info next_plot_info;
    std::vector<plot_info> all_plot_info;

    double all_plot_range[4]={1.0e100,-1.0e100,1.0e100,-1.0e100};

  };
  
  
  template<typename V>
    inline
    void ChartXY::AddPlot(const V &x, const V &y)
  {
    
    int N=x.size();

    vtkSmartPointer<vtkDoubleArray> X;
    vtkSmartPointer<vtkDoubleArray> Y;
    vtkSmartPointer<vtkTable> table;

    auto line=chartxy->GetPlot(num_plot);
    if (!line)
    {  
      X=vtkSmartPointer<vtkDoubleArray>::New();
      Y=vtkSmartPointer<vtkDoubleArray>::New();
      X->SetName("X");
      Y->SetName("Y");
      table=vtkSmartPointer<vtkTable>::New();
      table->AddColumn(X);
      table->AddColumn(Y);
      
      line=chartxy->AddPlot(vtkChartXY::LINE);
      line->SetInputData(table, 0, 1);
    }
    else
      table=line->GetInput();

    table->SetNumberOfRows(N);
    
    assert(x.size()==y.size());
    double xmin=1.0e100,xmax=-1.0e100;
    double ymin=1.0e100,ymax=-1.0e100;
    
    for (int i=0; i<N; i++)
    {
      table->SetValue(i,0,x[i]);
      table->SetValue(i,1,y[i]);
      xmin=std::min(xmin,x[i]);
      ymin=std::min(ymin,y[i]);
      xmax=std::max(xmax,x[i]);
      ymax=std::max(ymax,y[i]);
    }

    all_plot_range[0]=std::min(xmin,all_plot_range[0]);
    all_plot_range[1]=std::max(xmax,all_plot_range[1]);
    all_plot_range[2]=std::min(ymin,all_plot_range[2]);
    all_plot_range[3]=std::max(ymax,all_plot_range[3]);
    
    line->Modified();
    table->Modified();
    AddPlot();
  }
  

}

#endif
