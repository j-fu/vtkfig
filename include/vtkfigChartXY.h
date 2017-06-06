#ifndef VTKFIG_CHARTXY_H
#define VTKFIG_CHARTXY_H

#include "vtkDoubleArray.h"
#include "vtkChartXY.h"
#include "vtkTable.h"
#include "vtkPlot.h"
#include "vtkPointData.h"
#include "vtkContextActor.h"


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

    virtual std::string SubClassName() {return std::string("ChartXY");}

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
    void ServerRTSend(vtkSmartPointer<internals::Communicator> communicator);

    void ClientMTReceive(vtkSmartPointer<internals::Communicator> communicator); 


    void AddPlot();

    virtual void RTBuildVTKPipeline();


    
    vtkSmartPointer<vtkChartXY> chartxy;
    vtkSmartPointer<vtkContextActor> chartxyactor;
 
    static const int desclen=4;
    std::string title="test";
    // has to consist of doubles only because of endianness
    struct plot_data
    {
      vtkSmartPointer<vtkDoubleArray> X;
      vtkSmartPointer<vtkDoubleArray> Y;
      vtkSmartPointer<vtkTable> table;
      vtkSmartPointer<vtkPlot> line;
      plot_data(vtkSmartPointer<vtkChartXY> chartxy)
      {
        X=vtkSmartPointer<vtkDoubleArray>::New();
        Y=vtkSmartPointer<vtkDoubleArray>::New();
        X->SetName("X");
        Y->SetName("Y");
        table=vtkSmartPointer<vtkTable>::New();
        table->AddColumn(X);
        table->AddColumn(Y);
        line=chartxy->AddPlot(vtkChart::LINE);
        line->SetInputData(table, 0, 1);
      }
    };

/* line->SetColor(255, 0, 0, 255); */
/*  104   line = chart->AddPlot(vtkChart::LINE); */
/*  105   line->SetTable(table, 0, 2); */
/*  106   line->SetColor(0, 255, 0, 255); */
/*  107   line->SetWidth(2.0); */
    
    plot_data& get_plot_data(size_t nplot)
    {
      if (nplot>=all_plot_data.size())
      {
        all_plot_data.emplace_back(chartxy);

      }
      return all_plot_data[nplot];
    }

    struct plot_info
    {
      double line_type[desclen]={'-',0,0,0};
      double line_rgb[3]={0,0,0};
      plot_info(){};
      ~plot_info(){};
    };
    
    plot_info next_plot_info;
    std::vector<plot_data> all_plot_data;
    std::vector<plot_info> all_plot_info;

    double all_plot_range[4]={1.0e100,-1.0e100,1.0e100,-1.0e100};

  };
  
  
  template<typename V>
    inline
    void ChartXY::AddPlot(const V &x, const V &y)
  {

    auto plot=get_plot_data(num_plot);
    plot.X->Initialize();
    plot.Y->Initialize();
    int N=x.size();

    plot.table->SetNumberOfRows(N);

    assert(x.size()==y.size());
    double xmin=1.0e100,xmax=-1.0e100;
    double ymin=1.0e100,ymax=-1.0e100;

    for (int i=0; i<N; i++)
    {
      plot.table->SetValue(i,0,x[i]);
      plot.table->SetValue(i,1,y[i]);
      xmin=std::min(xmin,x[i]);
      ymin=std::min(ymin,y[i]);
      xmax=std::max(xmax,x[i]);
      ymax=std::max(ymax,y[i]);
    }

    all_plot_range[0]=std::min(xmin,all_plot_range[0]);
    all_plot_range[1]=std::max(xmax,all_plot_range[1]);
    all_plot_range[2]=std::min(ymin,all_plot_range[2]);
    all_plot_range[3]=std::max(ymax,all_plot_range[3]);
    
    AddPlot();;
  }
  

}

#endif
