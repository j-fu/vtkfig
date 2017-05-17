#ifndef VTKFIG_CONTOUR3D_H
#define VTKFIG_CONTOUR3D_H

#include "vtkRectilinearGrid.h"
#include "vtkRectilinearGridGeometryFilter.h"
#include "vtkFloatArray.h"
#include "vtkContourFilter.h"

#include "vtkfigFigure.h"
#include "vtkfigContour3DBase.h"
#include "vtkfigTools.h"

namespace vtkfig
{


///////////////////////////////////////////
  class Contour3D: public Contour3DBase
  {
  public:
    
    Contour3D();
    static std::shared_ptr<Contour3D> New() { return std::make_shared<Contour3D>(); }
    virtual std::string SubClassName() {return std::string("Contour3D");}
    
    template<typename V>
      void SetGrid(const V &xcoord, 
                   const V &ycoord, 
                   const V &zcoord);
    
    template<typename V>
      void UpdateValues(const V &values);
    
  private:


    vtkSmartPointer<vtkRectilinearGridGeometryFilter> geometry;
    
    vtkSmartPointer<vtkRectilinearGrid> gridfunc;
    vtkSmartPointer<vtkFloatArray> xcoord;
    vtkSmartPointer<vtkFloatArray> ycoord;
    vtkSmartPointer<vtkFloatArray> zcoord;
    vtkSmartPointer<vtkFloatArray> values;
    
    unsigned int Nx;
    unsigned int Ny;
    unsigned int Nz;
    int grid_changed=0;
    bool has_data=false;
    
    virtual void RTBuild(
      vtkSmartPointer<vtkRenderWindow> window,
      vtkSmartPointer<vtkRenderWindowInteractor> interactor,
      vtkSmartPointer<vtkRenderer> renderer);

    void ServerRTSend(vtkSmartPointer<Communicator> communicator);
    void ClientMTReceive(vtkSmartPointer<Communicator> communicator);
    
  };
  
  template<typename V>
    inline
    void Contour3D::SetGrid(const V &x, const V &y, const V &z)
  {
    Nx = x.size();
    Ny = y.size();
    Nz = z.size();
    
    
    xcoord->SetNumberOfComponents(1);
    xcoord->SetNumberOfTuples(Nx);
    
    ycoord->SetNumberOfComponents(1);
    ycoord->SetNumberOfTuples(Ny);
    
    zcoord->SetNumberOfComponents(1);
    zcoord->SetNumberOfTuples(Nz);
    
    for (int i=0; i<Nx; i++)
      xcoord->InsertComponent(i, 0, x[i]);
    for (int i=0; i<Ny; i++)
      ycoord->InsertComponent(i, 0, y[i]);
    for (int i=0; i<Nz; i++)
      zcoord->InsertComponent(i, 0, z[i]);
    
    gridfunc->SetDimensions(Nx, Ny, Nz);
    grid_changed=1;
    has_data=true;
    
    
  }
  
  template<typename V>
  inline
    void Contour3D::UpdateValues(const V&z)
  {
    double vmax=-1.0e100;
    double vmin=1.0e100;
    
    
    for (int k = 0, l=0; k < Nz; k++)
      for (int j = 0; j < Ny; j++)
        for (int i = 0; i < Nx; i++)
        {
          double v=z[l];
          vmin=std::min(v,vmin);
          vmax=std::max(v,vmax);
          values->InsertComponent(l++, 0, v);
        }
    
    values->Modified();
    SetVMinMax(vmin,vmax);
  }
  
  
}

#endif
