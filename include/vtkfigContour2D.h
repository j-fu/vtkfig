#ifndef VTKFIG_CONTOUR2D_H
#define VTKFIG_CONTOUR2D_H

#include "vtkRectilinearGrid.h"
#include "vtkFloatArray.h"
#include "vtkContourFilter.h"

#include "vtkfigFigure.h"
#include "vtkfigContour2DBase.h"
#include "vtkfigTools.h"


namespace vtkfig
{

  
  ///////////////////////////////////////////
  class Contour2D: public Contour2DBase
  {
  public:
    
    Contour2D();
    
    static std::shared_ptr<Contour2D> New() { return std::make_shared<Contour2D>(); }
    
    template<typename V>
      void SetGrid(const V &xcoord, 
                   const V &ycoord);
    
    template<typename V> void UpdateValues(const V &values);

    
    
  private:
    virtual void RTBuild(
      vtkSmartPointer<vtkRenderWindow> window,
      vtkSmartPointer<vtkRenderWindowInteractor> interactor,
      vtkSmartPointer<vtkRenderer> renderer);

    

    vtkSmartPointer<vtkFloatArray> xcoord;
    vtkSmartPointer<vtkFloatArray> ycoord;
    vtkSmartPointer<vtkFloatArray> values;
    
    unsigned int Nx;
    unsigned int Ny;
    
  };


  template<typename V>
  inline
  void Contour2D::SetGrid(const V &x, const V &y)
  {
    xcoord->Initialize();
    ycoord->Initialize();
    values->Initialize();

    Nx = x.size();
    Ny = y.size();
    
    xcoord->SetNumberOfComponents(1);
    xcoord->SetNumberOfTuples(Nx);
    
    ycoord->SetNumberOfComponents(1);
    ycoord->SetNumberOfTuples(Ny);

    double xmin=x[0];
    double xmax=x[Nx-1];

    double ymin=y[0];
    double ymax=y[Ny-1];

    
    
    for (int i=0; i<Nx; i++)
      xcoord->InsertComponent(i, 0, x[i]);
    for (int i=0; i<Ny; i++)
      ycoord->InsertComponent(i, 0, y[i]);


    values->SetNumberOfComponents(1);
    values->SetNumberOfTuples(Nx*Ny);
  }

  template<typename V>
  inline
  void Contour2D::UpdateValues(const V&z)
  {
    double vmax=-1.0e100;
    double vmin=1.0e100;

    
    for (int j = 0, k=0; j < Ny; j++)
      for (int i = 0; i < Nx; i++)
      {
        double v=z[j*Nx+i];
        vmin=std::min(v,vmin);
        vmax=std::max(v,vmax);
        values->InsertComponent(k++, 0, v);
      }
    
    values->Modified();
    SetVMinMax(vmin,vmax);
  }

}
#endif

