#include <memory>
#include <thread>

#include "vtkFloatArray.h"
#include "vtkSmartPointer.h"
#include "vtkRectilinearGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkXYPlotActor.h"



namespace visvtk
{
  
  class Communicator;
  
  class Plot
  {
  public:
    virtual vtkSmartPointer<vtkProp> GetActor()=0;
  };

  class Figure
  {
  public:
    Figure();
    ~Figure();
    
    void Show(Plot &plot);
    
    void Dump(std::string fname);
    
    void Clear(void);
    
  private:
    void Terminate(void);
    vtkSmartPointer<Communicator> communicator;
    std::shared_ptr<std::thread> render_thread;
  };
  
  

  class XYPlot: public Plot
  {
    
  public:
    
    vtkSmartPointer<vtkProp> GetActor()    {   return xyplot;    }
    XYPlot();
    
    
    void Reset(void);
    
    template<typename V> 
    void Add(const V &x, 
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
    
    
    void Add(vtkSmartPointer<vtkFloatArray> xVal,
             vtkSmartPointer<vtkFloatArray> yVal, 
             const double col[3],
             const std::string linespec);
    
  private:
    
    vtkSmartPointer<vtkXYPlotActor>  xyplot;
    int plot_no=0;
    vtkSmartPointer<vtkRectilinearGrid> curves[10];
  };
  
}


