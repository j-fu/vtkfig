#ifndef VTKFIG_CONTOUR3D_H
#define VTKFIG_CONTOUR3D_H

#include "vtkRectilinearGrid.h"
#include "vtkRectilinearGridGeometryFilter.h"
#include "vtkGeometryFilter.h"
#include "vtkUnstructuredGridGeometryFilter.h"
#include "vtkFloatArray.h"
#include "vtkContourFilter.h"

#include "vtkfigFigure.h"
#include "vtkfigContourBase.h"
#include "vtkfigTools.h"

namespace vtkfig
{


///////////////////////////////////////////
  class RectContour: public ContourBase
  {
  public:
  RectContour(): ContourBase()
    {
      
      xcoord = vtkSmartPointer<vtkFloatArray>::New();
      ycoord =vtkSmartPointer<vtkFloatArray>::New();
      zcoord =vtkSmartPointer<vtkFloatArray>::New();
      gridvalues = vtkSmartPointer<vtkFloatArray>::New();
      gridfunc=vtkSmartPointer<vtkRectilinearGrid>::New();
      gridfunc->GetPointData()->SetScalars(gridvalues);
    }
    static std::shared_ptr<RectContour> New() { return std::make_shared<RectContour>(); }
    virtual std::string SubClassName() {return std::string("RectContour");}
    
    template<typename V>
      void SetGrid(const V &xcoord, 
                   const V &ycoord, 
                   const V &zcoord);

    template<typename V>
      void SetGrid(const V &xcoord, 
                   const V &ycoord);
    
    template<typename V>
      void UpdateValues(const V &values);
    
  private:


    vtkSmartPointer<vtkRectilinearGridGeometryFilter> geometry;
    
    vtkSmartPointer<vtkRectilinearGrid> gridfunc;
    vtkSmartPointer<vtkFloatArray> gridvalues;
    vtkSmartPointer<vtkFloatArray> xcoord;
    vtkSmartPointer<vtkFloatArray> ycoord;
    vtkSmartPointer<vtkFloatArray> zcoord;
    
    unsigned int Nx;
    unsigned int Ny;
    unsigned int Nz;
    
    void RTBuild(
      vtkSmartPointer<vtkRenderWindow> window,
      vtkSmartPointer<vtkRenderWindowInteractor> interactor,
      vtkSmartPointer<vtkRenderer> renderer)
    {
      if (state.spacedim==2)
        ContourBase::RTBuild2D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>(window, interactor,renderer,gridfunc);
      else
        ContourBase::RTBuild3D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>(window, interactor,renderer,gridfunc);
    }

    void ServerRTSend(vtkSmartPointer<Communicator> communicator)
    {
      ContourBase::ServerRTSend(communicator, gridfunc, gridvalues);
    }

    void ClientMTReceive(vtkSmartPointer<Communicator> communicator)
    {
      ContourBase::ClientMTReceive(communicator, gridfunc, gridvalues);
    }

    
  };

  template<typename V>
  inline
    void RectContour::SetGrid(const V &x, const V &y)
  {
    assert(state.spacedim!=3);
    /// on dimension change, we shall re-create gridfunc
    state.spacedim=2;
    xcoord->Initialize();
    ycoord->Initialize();
    gridvalues->Initialize();
    gridfunc->SetXCoordinates(xcoord);
    gridfunc->SetYCoordinates(ycoord);
    
    Nx = x.size();
    Ny = y.size();
    Nz = 0;

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
    
    gridfunc->SetDimensions(Nx, Ny, 1);
    gridvalues->SetNumberOfTuples(Nx*Ny);
    grid_changed=1;
    has_data=true;
  }
  
  
  template<typename V>
    inline
    void RectContour::SetGrid(const V &x, const V &y, const V &z)
  {
    state.spacedim=3;
        
    Nx = x.size();
    Ny = y.size();
    Nz = z.size();
    gridfunc->SetXCoordinates(xcoord);
    gridfunc->SetYCoordinates(ycoord);
    gridfunc->SetZCoordinates(zcoord);
    
    
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
    gridvalues->SetNumberOfTuples(Nx*Ny*Nz);
    grid_changed=1;
    has_data=true;
    
    
  }
  
  template<typename V>
  inline
    void RectContour::UpdateValues(const V&values)
  {

    double vmin=1.0e100;
    double vmax=-1.0e100;
    int npoints=values.size();
    assert(npoints==gridvalues->GetNumberOfTuples());
    for (int i=0;i<npoints; i++)
    {
      double v=values[i];
      vmin=std::min(v,vmin);
      vmax=std::max(v,vmax);
      gridvalues->InsertComponent(i,0,v);
    }
    gridvalues->Modified();
    SetVMinMax(vmin,vmax);
  }
  
  
}

#endif
