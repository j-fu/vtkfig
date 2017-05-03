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
    

    void Title(const char *title);
    
    template<typename V> 
      void AddPlot(const V &x, 
               const V &y, 
               const double col[3],
               const std::string linespec);
    
    void Clear();
  private:

    void AddPlot(const vtkSmartPointer<vtkFloatArray> xVal,
                 const vtkSmartPointer<vtkFloatArray> yVal, 
                 const double col[3],
                 const std::string linespec);
    
    void RTBuild();
    void Init();
    
    vtkSmartPointer<vtkXYPlotActor> xyplot;
    int iplot=0;
  };
  
  
  template<typename V>
    inline
    void XYPlot::AddPlot(const V &x, 
                         const V &y, 
                         const double col[3],
                         const std::string linespec)
  {
    vtkSmartPointer<vtkFloatArray> xVal= vtkSmartPointer<vtkFloatArray>::New();
    vtkSmartPointer<vtkFloatArray> yVal= vtkSmartPointer<vtkFloatArray>::New();
    int N=x.size();
    for (int i=0; i<N; i++)
    {
      xVal->InsertNextTuple1(x[i]);
      yVal->InsertNextTuple1(y[i]);
    }
    AddPlot(xVal,yVal,col, linespec);
  }
  

}

#endif
