#ifndef VTKFIG_XYPLOT_H
#define VTKFIG_XYPLOT_H

#include "vtkFloatArray.h"
#include "vtkXYPlotActor.h"

#include "vtkfigFigure.h"

namespace vtkfig
{
  
  class XYPlot: public Figure
  {
    
  public:
    XYPlot();
    
    void Title(const char *title);
    
    template<typename V> 
      void Add(const V &x, 
               const V &y, 
               const double col[3],
               const std::string linespec);
    
    
  private:
    void Add(const vtkSmartPointer<vtkFloatArray> xVal,
             const vtkSmartPointer<vtkFloatArray> yVal, 
             const double col[3],
             const std::string linespec);
    
    vtkSmartPointer<vtkXYPlotActor> xyplot;
    int iplot=0;
  };
  

  template<typename V>
    inline
    void XYPlot::Add(const V &x, 
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
    Add(xVal,yVal,col, linespec);
  }


}

#endif
