#ifndef VTKFIG_TRICONTOUR2D_H
#define VTKFIG_TRICONTOUR2D_H

#include "vtkGeometryFilter.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"
#include "vtkUnstructuredGrid.h"

#include "vtkfigContourBase.h"
#include "vtkfigTools.h"





namespace vtkfig
{

  
  class SimplexContour: public ContourBase
  {
  public:
    SimplexContour();
    
    static std::shared_ptr<SimplexContour> New() { return std::make_shared<SimplexContour>(); }

    virtual std::string SubClassName() {return std::string("SimplexContour");}
    
    template <class V, class IV>
      void SetGrid(int dim,
                   const V& points, 
                   const IV& cells);
    
    template <class V>
      void UpdateValues(const V&values);
    
  private:

    
    virtual void RTBuild(
      vtkSmartPointer<vtkRenderWindow> window,
      vtkSmartPointer<vtkRenderWindowInteractor> interactor,
      vtkSmartPointer<vtkRenderer> renderer)
    {
      
      if (state.spacedim==2)
        ContourBase::RTBuild2D<vtkUnstructuredGrid,vtkGeometryFilter>(window, interactor,renderer,gridfunc);
      else
        ContourBase::RTBuild3D<vtkUnstructuredGrid>(window,interactor,renderer,gridfunc);
    }
    
    void ServerRTSend(vtkSmartPointer<Communicator> communicator)
    {
      ContourBase::ServerRTSend(communicator, gridfunc, gridvalues);
    }

    void ClientMTReceive(vtkSmartPointer<Communicator> communicator)
    {
      ContourBase::ClientMTReceive(communicator, gridfunc, gridvalues);
    }
   
    
    vtkSmartPointer<vtkUnstructuredGrid> gridfunc;
    vtkSmartPointer<vtkFloatArray> gridvalues;
    vtkSmartPointer<vtkPoints> gridpoints;
  };
  


  inline SimplexContour::SimplexContour(): ContourBase()
  {
    gridfunc=vtkSmartPointer<vtkUnstructuredGrid>::New();
    gridvalues = vtkSmartPointer<vtkFloatArray>::New();
    gridpoints = vtkSmartPointer<vtkPoints>::New();
    gridfunc->SetPoints(gridpoints);
    gridfunc->GetPointData()->SetScalars(gridvalues);
  }
  
  
  
  template <class V, class IV>
    inline
    void SimplexContour::SetGrid(int dim, 
                                 const V& points, 
                                 const IV& cells)
  {
    state.spacedim=dim;
    int npoints=points.size()/state.spacedim;
    
    
    if (has_data)
    {
      gridfunc->Reset();
      gridpoints->Initialize();
      gridvalues->Initialize();
    }

    if (state.spacedim==2)
    {
      for (int icell=0;icell<cells.size(); icell+=3)
      {
        vtkIdType 	 c[3]={cells[icell+0],cells[icell+1],cells[icell+2]};
        gridfunc->InsertNextCell(VTK_TRIANGLE,3,c);
      }

      for (int ipoint=0;ipoint<points.size(); ipoint+=2)
      {
        gridpoints->InsertNextPoint(points[ipoint+0],points[ipoint+1],0);
      }
    }
    else
    {
      for (int icell=0;icell<cells.size(); icell+=4)
      {
        vtkIdType 	 c[4]={cells[icell+0],cells[icell+1],cells[icell+2],cells[icell+3]};
        gridfunc->InsertNextCell(VTK_TETRA,4,c);
      }
    
      for (int ipoint=0;ipoint<points.size(); ipoint+=3)
      {
        gridpoints->InsertNextPoint(points[ipoint+0],points[ipoint+1],points[ipoint+2]);
      }
    }    
    gridvalues->SetNumberOfComponents(1);
    gridvalues->SetNumberOfTuples(npoints);
    
    grid_changed=1;
    has_data=true;
  }
  
  template <class V>
    inline
    void SimplexContour::UpdateValues(const V& values)
  {
    double vmin=1.0e100;
    double vmax=-1.0e100;
    int npoints=values.size();
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
