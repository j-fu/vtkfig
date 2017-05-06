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
    
    template<typename V> 
      void AddPlot(const V &x, 
               const V &y, 
               const double col[3],
               const std::string linespec);
    
    void Clear();

    virtual void RTSend(vtkSmartPointer<Communicator> communicator) 
      {
        
        communicator->SendInt(num_plots);
        for (int i=0;i<num_plots;i++)
        {
          communicator->Send(xVal[i],1,1);
          communicator->Send(yVal[i],1,1);
        }
      };

      virtual void MTReceive(vtkSmartPointer<Communicator> communicator) 
      {
        Clear();
        int np;
        communicator->ReceiveInt(np);
        for (int i=0;i<np;i++)
        {
          vtkSmartPointer<vtkFloatArray> xVal= vtkSmartPointer<vtkFloatArray>::New();
          vtkSmartPointer<vtkFloatArray> yVal= vtkSmartPointer<vtkFloatArray>::New();
          communicator->Receive(xVal,1,1);
          communicator->Receive(yVal,1,1);
          double color[3]={0,1,0};
          AddPlot(xVal,yVal,color,"-");
        }
        
      };

  private:

    void AddPlot(const vtkSmartPointer<vtkFloatArray> xVal,
                 const vtkSmartPointer<vtkFloatArray> yVal, 
                 const double col[3],
                 const std::string linespec);
    
    void RTBuild();
    void Init();
    
    vtkSmartPointer<vtkXYPlotActor> xyplot;
    std::vector<vtkSmartPointer<vtkFloatArray>> xVal;
    std::vector<vtkSmartPointer<vtkFloatArray>> yVal;
    int num_plots=0;
    bool modified=false;
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
