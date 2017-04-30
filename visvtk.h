#ifndef VISVTK_H
#define VISVTK_H

#include <memory>
#include <thread>

#include "vtkFloatArray.h"
#include "vtkSmartPointer.h"

#include "vtkRectilinearGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkXYPlotActor.h"
#include "vtkPointData.h"




namespace visvtk
{
  
  class Communicator;
  class Plot;
  

  #define INTERACTOR_STYLE_2D 2
  #define INTERACTOR_STYLE_3D 3

  class Figure
  {
  public:
    Figure();

    ~Figure();
    
    void Dump(std::string fname);
    
    void Clear(void);
    
    void Show(Plot & plot);

    void ShowInteractive(Plot & plot);
    
    void SetInteractorStyle(int istyle);
    
  private:
    void Restart(void);
    void Start(void);
    void Terminate(void);
    vtkSmartPointer<Communicator> communicator;
    std::shared_ptr<std::thread> render_thread;
  };
  
  /// Base class for all plots.
  ///
  /// It justs consists of a set of instances of vtkActor which
  /// contains the data  used by vtk.
  /// 
  /// Derived classes just should fill these actors by calling Plot::AddActor
  ///
  /// In this way, any vtk rendering pipeline can be used. 
  /// 

  class Plot
  {
    friend class Figure;
  public:
    Plot();
    void SetBackground(double r, double g, double b) { bgcolor[0]=r; bgcolor[1]=g; bgcolor[2]=b;}
    bool IsEmpty();

    struct RGBPoint { double x,r,g,b;};
    typedef std::vector<RGBPoint> RGBTable;
    static vtkSmartPointer<vtkLookupTable>  BuildLookupTable(RGBTable & xrgb, int size);

  protected:
    void AddActor(vtkSmartPointer<vtkProp> prop);
    

  private:
    std::shared_ptr<std::vector<vtkSmartPointer<vtkProp>>>actors;
    double bgcolor[3]={1,1,1};
  };
  
  ///////////////////////////////////////////////////
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
    
    
    
  private:
    void Add(const vtkSmartPointer<vtkFloatArray> xVal,
             const vtkSmartPointer<vtkFloatArray> yVal, 
             const double col[3],
             const std::string linespec);

    vtkSmartPointer<vtkXYPlotActor> xyplot;
    int iplot=0;
  };

  ///////////////////////////////////////////
  class Contour2D: public Plot
    {
    public:
      
      Contour2D();
      
      template<typename V>
      void Add(const V &xcoord, 
               const V &ycoord, 
               const V &values);

      
      void ShowSurface(bool b) {show_surface=b;}
      void ShowContour(bool b) {show_contour=b;}
      void ShowSurfaceColorbar(bool b) {show_surface_colorbar=b;}
      void ShowContourColorbar(bool b) {show_contour_colorbar=b;}

      void SetSurfaceRGBTable(RGBTable & tab)
      {
        surface_lut=BuildLookupTable(tab,256);
      }
      void SetContourRGBTable(RGBTable & tab)
      {
        contour_lut=BuildLookupTable(tab,256);
      }
      
    private:
      void Add(const vtkSmartPointer<vtkFloatArray> xcoord,
               const vtkSmartPointer<vtkFloatArray> ycoord,
               const vtkSmartPointer<vtkFloatArray> values);

      vtkSmartPointer<vtkLookupTable> surface_lut;
      vtkSmartPointer<vtkLookupTable> contour_lut;

      bool show_surface=true;
      bool show_contour=true;
      bool show_surface_colorbar=true;
      bool show_contour_colorbar=false;
  };

  ///////////////////////////////////////////
  class Surf2D: public Plot
    {
    public:
      
      Surf2D();
      
      template<typename V>
      void Add(const V &xcoord, 
               const V &ycoord, 
               const V &values);
    private:
      void Add(vtkSmartPointer<vtkStructuredGrid> gridfunc, double Lxy, double Lz);
  };

  ///////////////////////////////////////////
  class Quiver2D: public Plot
    {
    public:
      
      Quiver2D();
      
      template<typename V>
      void Add(const V &x, 
               const V &y, 
               const V &u,
               const V &v);
      
    private:
      void Add(const vtkSmartPointer<vtkFloatArray> xcoord,
               const vtkSmartPointer<vtkFloatArray> ycoord,
               const vtkSmartPointer<vtkFloatArray> colors,
               const vtkSmartPointer<vtkFloatArray> values);

  };

  template<typename V>
  inline
  void Quiver2D::Add(const V &x, 
                     const V &y, 
                     const V &u,
                     const V &v)
  {
    const unsigned int Nx = x.size();
    const unsigned int Ny = x.size();

    vtkSmartPointer<vtkFloatArray> xcoord = vtkSmartPointer<vtkFloatArray>::New();
    xcoord->SetNumberOfComponents(1);
    xcoord->SetNumberOfTuples(Nx);

    vtkSmartPointer<vtkFloatArray> ycoord = vtkSmartPointer<vtkFloatArray>::New();
    ycoord->SetNumberOfComponents(1);
    ycoord->SetNumberOfTuples(Ny);
    
    for (int i=0; i<Nx; i++)
      xcoord->InsertComponent(i, 0, x[i]);
    for (int i=0; i<Ny; i++)
      ycoord->InsertComponent(i, 0, y[i]);
    
    // add magnitude of (u,v) as scalars to grid
    vtkSmartPointer<vtkFloatArray>colors = vtkSmartPointer<vtkFloatArray>::New();
    colors->SetNumberOfComponents(1);
    colors->SetNumberOfTuples(Nx*Ny);
    
    // add vector (u,v) to grid
    vtkSmartPointer<vtkFloatArray>vectors = vtkSmartPointer<vtkFloatArray>::New();
    vectors->SetNumberOfComponents(3);
    vectors->SetNumberOfTuples(Nx*Ny);
    
    for (int j = 0, k=0; j < Ny; j++)
      for (int i = 0; i < Nx; i++)
      {
        colors->InsertTuple1(k,sqrt(u[j*Nx+i]*u[j*Nx+i]+v[j*Nx+i]*v[j*Nx+i]));
        vectors->InsertTuple3(k, u[j*Nx+i], v[j*Nx+i], 0.0);
        k++;
      }
    
    Add(xcoord,ycoord,colors,vectors);

  }



  ///////////////////////////////////////////
  // inline template implementations
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

  template<typename V>
  inline
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
  inline
  void Surf2D::Add(const V &x, const V &y, const V &z)
  {
    const unsigned int Nx = x.size();
    const unsigned int Ny = x.size();
    int i,j,k;
    double Lxy,Lz;
    
    if (x[Nx-1]-x[0] > y[Ny-1]-y[0])
      Lxy = x[Nx-1]-x[0];
    else
      Lxy = y[Ny-1]-y[0];
    double z_low = 10000, z_upp = -10000;
    
    
    vtkSmartPointer<vtkStructuredGrid> 	    gridfunc= vtkSmartPointer<vtkStructuredGrid>::New();
    gridfunc->SetDimensions(Nx, Ny, 1);
    
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    for (j = 0; j < Ny; j++)
    {
      for (i = 0; i < Nx; i++)
      {
        points->InsertNextPoint(x[i], y[j], z[j*Nx+i]);
        
        if (z[j*Nx+i]< z_low)
          z_low = z[j*Nx+i];
        if (z[j*Nx+i]> z_upp)
          z_upp = z[j*Nx+i];
      }
    }
    gridfunc->SetPoints(points);
    
    
    vtkSmartPointer<vtkFloatArray> colors = vtkSmartPointer<vtkFloatArray>::New();
    colors->SetNumberOfComponents(1);
    colors->SetNumberOfTuples(Nx*Ny);
    k = 0;
    for (j = 0; j < Ny; j++)
      for (i = 0; i < Nx; i++)
      {
        colors->InsertComponent(k, 0, z[j*Nx+i]);
        k++;
      }
    
    gridfunc->GetPointData()->SetScalars(colors);
    
    Lz = z_upp-z_low;
    
    Add(gridfunc,Lxy,Lz);
    
  }

  
}


#endif
