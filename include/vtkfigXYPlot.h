#ifndef VTKFIG_XYPLOT_H
#define VTKFIG_XYPLOT_H

#include "vtkDoubleArray.h"
#include "vtkXYPlotActor.h"
#include "vtkRectilinearGrid.h"
#include "vtkPointData.h"
#include "vtkXYPlotWidget.h"


#include "vtkfigFigure.h"

namespace vtkfig
{
  
  /// 
  /// 1D function plot.
  ///
  class XYPlot: public Figure
  {
    
  public:
    XYPlot();
    static std::shared_ptr<XYPlot> New() { return std::make_shared<XYPlot>(); }

    virtual std::string SubClassName() {return std::string("XYPlot");}

    void SetTitle(const char *t) {xyplot->SetTitle(t);    title=t;}
    void SetXTitle(const char *t) {xyplot->SetTitle(t);    xtitle=t;}
    void SetYTitle(const char *t) {xyplot->SetTitle(t);    ytitle=t;}
    void SetXRange(double x0, double x1) {fixXMin=x0; fixXMax=x1;}
    void SetYRange(double y0, double y1) {fixYMin=y0; fixYMax=y1;}
    void SetNumberOfXLabels(int n) {nxlabels=n;}
    void SetNumberOfYLabels(int n) {nylabels=n;}


    template <typename T> 
    void LineType(const T *type) 
    {
      for (int i=0;i<desclen;i++) 
      {
        next_plot_info.line_type[i]=static_cast<double>(type[i]);
        if (type[i]=='\0') break;
      }
    }

    template <typename T> 
    void Legend(const T *type) 
    {
      for (int i=0;i<desclen;i++) 
      {
        next_plot_info.legend[i]=static_cast<double>(type[i]);
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

    vtkSmartPointer<vtkXYPlotWidget> widget;

    void AddPlot();
    
  
    virtual void RTBuildVTKPipeline();

  
    
    vtkSmartPointer<vtkXYPlotActor> xyplot;

 
    static const int desclen=12;
    std::string title="";
    std::string ytitle="y";
    std::string xtitle="x";

    // has to consist of doubles only because of endianness
    struct plot_data
    {
      vtkSmartPointer<vtkDoubleArray> X;
      vtkSmartPointer<vtkDoubleArray> Y;
      vtkSmartPointer<vtkRectilinearGrid> curve;
      plot_data(vtkSmartPointer<vtkXYPlotActor> xyplot)
      {
        X=vtkSmartPointer<vtkDoubleArray>::New();
        Y=vtkSmartPointer<vtkDoubleArray>::New();
        curve=vtkSmartPointer<vtkRectilinearGrid>::New();
        curve->SetXCoordinates(X);
        curve->GetPointData()->SetScalars(Y);
        xyplot->AddDataSetInput(curve);
      }
    };
    
    plot_data& get_plot_data(size_t nplot)
    {
      if (nplot>=all_plot_data.size())
        all_plot_data.emplace_back(xyplot);
      return all_plot_data[nplot];
    }

    struct plot_info
    {
      double line_type[desclen]={'-',0};
      double legend[desclen]={0};
      double line_rgb[3]={0,0,0};
      plot_info(){};
      ~plot_info(){};
    };
    
    plot_info next_plot_info;
    std::vector<plot_data> all_plot_data;
    std::vector<plot_info> all_plot_info;

    double dynXMin=1.0e100,dynXMax=-1.0e100,dynYMin=1.0e100,dynYMax=-1.0e100;
    double fixXMin=1.0e100,fixXMax=-1.0e100,fixYMin=1.0e100,fixYMax=-1.0e100;
    int nxlabels=5;
    int nylabels=5;
        
  };
  
  
  template<typename V>
    inline
  void XYPlot::AddPlot(const V &x, const V &y)
  {

    auto plot=get_plot_data(num_plot);
    plot.X->Initialize();
    plot.Y->Initialize();
    

    int N=x.size();
    assert(x.size()==y.size());
    double xmin=1.0e100,xmax=-1.0e100;
    double ymin=1.0e100,ymax=-1.0e100;

    for (int i=0; i<N; i++)
    {
      plot.X->InsertNextTuple1(x[i]);
      plot.Y->InsertNextTuple1(y[i]);
      xmin=std::min(xmin,x[i]);
      ymin=std::min(ymin,y[i]);
      xmax=std::max(xmax,x[i]);
      ymax=std::max(ymax,y[i]);
    }

    dynXMin=std::min(xmin,dynXMin);
    dynXMax=std::max(xmax,dynXMax);
    dynYMin=std::min(ymin,dynYMin);
    dynYMax=std::max(ymax,dynYMax);
    
    AddPlot();;
  }
  

}

#endif
