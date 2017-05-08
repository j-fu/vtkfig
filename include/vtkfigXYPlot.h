#ifndef VTKFIG_XYPLOT_H
#define VTKFIG_XYPLOT_H

#include "vtkFloatArray.h"
#include "vtkXYPlotActor.h"
#include "vtkRectilinearGrid.h"

#include "vtkfigFigure.h"

namespace vtkfig
{
  
  class XYPlot: public Figure
  {
    
  public:
    XYPlot();
    static std::shared_ptr<XYPlot> New() { return std::make_shared<XYPlot>(); }

    virtual std::string SubClassName() {return std::string("XYPlot");}

    void Title(const char *title);

    template <typename T> 
    void LineType(const T *type) 
    { for (int i=0;i<desclen;i++) 
      {
        next_plot_info.line_type[i]=static_cast<float>(type[i]);
        if (type[i]=='\0') break;
      }
    }
    void LineColorRGB(float r, float g, float b) { next_plot_info.line_rgb[0]=r; next_plot_info.line_rgb[1]=g; next_plot_info.line_rgb[2]=b;}

    void LineColorRGB(float rgb[3]) { next_plot_info.line_rgb[0]=rgb[0]; next_plot_info.line_rgb[1]=rgb[1]; next_plot_info.line_rgb[2]=rgb[2];}
    
    template<typename V>  void AddPlot(const V &x,  const V &y);
    
    void Clear();



  private:

    void ServerRTSend(vtkSmartPointer<Communicator> communicator);

    void ClientMTReceive(vtkSmartPointer<Communicator> communicator); 


    void AddPlot(const vtkSmartPointer<vtkFloatArray> xVal,
                 const vtkSmartPointer<vtkFloatArray> yVal);
    
    void RTBuild();
    void Init();
    
    vtkSmartPointer<vtkXYPlotActor> xyplot;
    std::vector<vtkSmartPointer<vtkFloatArray>> xVal;
    std::vector<vtkSmartPointer<vtkFloatArray>> yVal;

    static const int desclen=4;
    std::string title="test";
    // has to consist of floats only because of endianness
    struct plot_info
    {
      float line_type[desclen]={'-',0,0,0};
      float line_rgb[3]={0,0,0};
      plot_info(){};
    };
    
    plot_info next_plot_info;
    std::vector<plot_info> all_plot_info;
  };
  
  
  template<typename V>
    inline
  void XYPlot::AddPlot(const V &x, const V &y)
  {
    vtkSmartPointer<vtkFloatArray> xVal= vtkSmartPointer<vtkFloatArray>::New();
    vtkSmartPointer<vtkFloatArray> yVal= vtkSmartPointer<vtkFloatArray>::New();
    int N=x.size();
    for (int i=0; i<N; i++)
    {
      xVal->InsertNextTuple1(x[i]);
      yVal->InsertNextTuple1(y[i]);
    }
    AddPlot(xVal,yVal);
  }
  

}

#endif
