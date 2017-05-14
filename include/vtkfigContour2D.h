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
    
    template<typename V>
      void UpdateValues(const V &values);
    
    
    
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
    Nx = x.size();
    Ny = y.size();
    
    xcoord = vtkSmartPointer<vtkFloatArray>::New();
    xcoord->SetNumberOfComponents(1);
    xcoord->SetNumberOfTuples(Nx);
    
    ycoord =vtkSmartPointer<vtkFloatArray>::New();
    ycoord->SetNumberOfComponents(1);
    ycoord->SetNumberOfTuples(Ny);
    
    for (int i=0; i<Nx; i++)
      xcoord->InsertComponent(i, 0, x[i]);
    for (int i=0; i<Ny; i++)
      ycoord->InsertComponent(i, 0, y[i]);
    
    values = vtkSmartPointer<vtkFloatArray>::New();
    values->SetNumberOfComponents(1);
    values->SetNumberOfTuples(Nx*Ny);
    
    for (int j = 0, k=0; j < Ny; j++)
      for (int i = 0; i < Nx; i++)
        values->InsertComponent(k++, 0,0);
  }

  template<typename V>
  inline
  void Contour2D::UpdateValues(const V&z)
  {
    vmax=-1.0e100;
    vmin=1.0e100;

    
    for (int j = 0, k=0; j < Ny; j++)
      for (int i = 0; i < Nx; i++)
      {
        double v=z[j*Nx+i];
        vmin=std::min(v,vmin);
        vmax=std::max(v,vmax);
        values->InsertComponent(k++, 0, v);
      }

    values->Modified();
    surface_lut->SetTableRange(vmin,vmax);
    surface_lut->Modified();
    contour_lut->SetTableRange(vmin,vmax);
    contour_lut->Modified();


    double tempdiff = (vmax-vmin)/(10*ncont);
    isocontours->GenerateValues(ncont, vmin+tempdiff, vmax-tempdiff);
    
  }

}
#endif

