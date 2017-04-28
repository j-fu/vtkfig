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
  
  class Plot
  {
  public:
    std::shared_ptr<std::vector<vtkSmartPointer<vtkProp>>>actors;
    Plot(): actors(std::make_shared<std::vector<vtkSmartPointer<vtkProp>>>()) {};
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
    
    void Title(const char *title);
    
    void Add(vtkSmartPointer<vtkFloatArray> xVal,
             vtkSmartPointer<vtkFloatArray> yVal, 
             const double col[3],
             const std::string linespec);
    
  private:
    std::shared_ptr<std::vector<vtkSmartPointer<vtkRectilinearGrid>>> curves;
    vtkSmartPointer<vtkXYPlotActor> xyplot;
  };


  class Contour2D: public Plot
    {
    public:

      Contour2D();

      
      template<typename Vec_t>
      void Set(const Vec_t &x, const Vec_t &y, const Vec_t &z)
	{
          const unsigned int Nx = x.size();
          const unsigned int Ny = x.size();
          unsigned int i, j, k;
          
          vtkSmartPointer<vtkFloatArray> xcoord = vtkSmartPointer<vtkFloatArray>::New();
          xcoord->SetNumberOfComponents(1);
          xcoord->SetNumberOfTuples(Nx);

          vtkSmartPointer<vtkFloatArray> ycoord =vtkSmartPointer<vtkFloatArray>::New();
          ycoord->SetNumberOfComponents(1);
          ycoord->SetNumberOfTuples(Ny);
          
          for (i=0; i<Nx; i++)
            xcoord->InsertComponent(i, 0, x[i]);
          for (i=0; i<Ny; i++)
            ycoord->InsertComponent(i, 0, y[i]);
          
          // add z-values as scalars to grid
          vtkSmartPointer<vtkFloatArray>values = vtkSmartPointer<vtkFloatArray>::New();
          values->SetNumberOfComponents(1);
          values->SetNumberOfTuples(Nx*Ny);
          k = 0;
          for (j = 0; j < Ny; j++)
            for (i = 0; i < Nx; i++)
            {
              values->InsertComponent(k, 0, z[j*Nx+i]);
              k++;
            }
          Set(xcoord,ycoord, values);

	}
      void Set(vtkSmartPointer<vtkFloatArray> xcoord ,vtkSmartPointer<vtkFloatArray> ycoord ,vtkSmartPointer<vtkFloatArray> values );  

    private:
      vtkSmartPointer<vtkActor> outline;
      vtkSmartPointer<vtkActor> surfplot;
      vtkSmartPointer<vtkActor> contours;
      vtkSmartPointer<vtkScalarBarActor> colorbar;
  };

}


