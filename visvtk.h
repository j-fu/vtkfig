#include <memory>
#include <thread>

#include "vtkFloatArray.h"
#include "vtkSmartPointer.h"

#include "vtkRectilinearGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkXYPlotActor.h"

#include "vtkActor.h"
#include "vtkScalarBarActor.h"



namespace visvtk
{
  
  class Communicator;
  class Plot;
  
  class Figure
  {
  public:
    Figure();

    ~Figure();
    
    void Dump(std::string fname);
    
    void Clear(void);
    
    void Show(Plot & plot);
    
  private:
    void Terminate(void);
    vtkSmartPointer<Communicator> communicator;
    std::shared_ptr<std::thread> render_thread;
  };
  
  /// Base class for all plots.
  ///
  /// It justs consists of a set of instances of vtkActor which
  /// contains the data  used by vtk.
  /// 
  /// Derived classes just should fill these actors
  ///

  class Plot
  {
    friend class Figure;
  public:
    Plot();
  protected:
    std::shared_ptr<std::vector<vtkSmartPointer<vtkProp>>>actors;
  };


  /// 
  /// Set  xy plots
  /// 
  class XYPlot: public Plot
  {
    
  public:
    XYPlot();

    void Title(const char *title);

    template<typename V> 
    void Add(const V &x, 
             const V &y, 
             const double col[3],
             const std::string linespec);
    
    
    void Add(const vtkSmartPointer<vtkFloatArray> xVal,
             const vtkSmartPointer<vtkFloatArray> yVal, 
             const double col[3],
             const std::string linespec);
    
  private:
    vtkSmartPointer<vtkXYPlotActor> xyplot;
    int iplot=0;
  };


  class Contour2D: public Plot
    {
    public:
      
      Contour2D();
      
      template<typename V>
      void Add(const V &xcoord, 
               const V &ycoord, 
               const V &values);
      
      void Add(const vtkSmartPointer<vtkFloatArray> xcoord,
               const vtkSmartPointer<vtkFloatArray> ycoord,
               const vtkSmartPointer<vtkFloatArray> values);
      
  };


  

  template<typename V>
  void Contour2D::Add(const V &x, const V &y, const V &z)
  {
    const unsigned int Nx = x.size();
    const unsigned int Ny = x.size();
    
    vtkSmartPointer<vtkFloatArray> xcoord = vtkSmartPointer<vtkFloatArray>::New();
    xcoord->SetNumberOfComponents(1);
    xcoord->SetNumberOfTuples(Nx);
    
    vtkSmartPointer<vtkFloatArray> ycoord =vtkSmartPointer<vtkFloatArray>::New();
    ycoord->SetNumberOfComponents(1);
    ycoord->SetNumberOfTuples(Ny);
    
    for (int i=0; i<Nx; i++)
      xcoord->InsertComponent(i, 0, x[i]);
    for (int i=0; i<Ny; i++)
      ycoord->InsertComponent(i, 0, y[i]);
    
    vtkSmartPointer<vtkFloatArray>values = vtkSmartPointer<vtkFloatArray>::New();
    values->SetNumberOfComponents(1);
    values->SetNumberOfTuples(Nx*Ny);
    
    for (int j = 0, k=0; j < Ny; j++)
      for (int i = 0; i < Nx; i++)
        values->InsertComponent(k++, 0, z[j*Nx+i]);
    
    Add(xcoord,ycoord, values);
    
  }
  
  
  template<typename V> 
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


