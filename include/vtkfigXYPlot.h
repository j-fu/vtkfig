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
    void LineType(const char *type) { strncpy(line_type,type,16);}
    void LineColorRGB(float r, float g, float b) { line_rgb[0]=r; line_rgb[1]=g; line_rgb[2]=b;}
    
    template<typename V> 
    void AddPlot(const V &x,  const V &y);
    
    void Clear();

    void ServerRTSend(vtkSmartPointer<Communicator> communicator);
    void ClientMTReceive(vtkSmartPointer<Communicator> communicator); 


  private:

    void AddPlot(const vtkSmartPointer<vtkFloatArray> xVal,
                 const vtkSmartPointer<vtkFloatArray> yVal);
    
    void RTBuild();
    void Init();
    
    vtkSmartPointer<vtkXYPlotActor> xyplot;
    std::vector<vtkSmartPointer<vtkFloatArray>> xVal;
    std::vector<vtkSmartPointer<vtkFloatArray>> yVal;
    int num_plots=0;
    float line_rgb[3]={0,0,0};
    char line_type[16]={'-','\0'};
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
