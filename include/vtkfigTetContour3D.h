#ifndef VTKFIG_TETCONTOUR3D_H
#define VTKFIG_TETCONTOUR3D_H

#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"

#include "vtkfigContour3DBase.h"
#include "vtkfigTools.h"



#include <vtkUnstructuredGrid.h>


namespace vtkfig
{
  
  
  class TetContour3D: public Contour3DBase
  {
  public:
    
    TetContour3D();
    static std::shared_ptr<TetContour3D> New() { return std::make_shared<TetContour3D>(); }
    virtual std::string SubClassName() {return std::string("TetContour3D");}
    
    template <class V, class IV>
      void SetGrid(const V& points, 
                   const IV& cells);
    
    template <class V>
      void UpdateValues(const V&values);
    
  private:
    
    
    virtual void RTBuild(
      vtkSmartPointer<vtkRenderWindow> window,
      vtkSmartPointer<vtkRenderWindowInteractor> interactor,
      vtkSmartPointer<vtkRenderer> renderer);
    
    void ServerRTSend(vtkSmartPointer<Communicator> communicator);
    void ClientMTReceive(vtkSmartPointer<Communicator> communicator);
    
    
    vtkSmartPointer<vtkUnstructuredGrid> gridfunc;
    vtkSmartPointer<vtkFloatArray> gridvalues;
    vtkSmartPointer<vtkPoints> gridpoints;
    bool has_data=false;
    int grid_changed=0;
  };
  

  

  
  
  template <class V, class IV>
    inline
    void TetContour3D::SetGrid(const V& points, 
                               const IV& cells)
  {
    int npoints=points.size()/3;
//    int ncells=cells.size()/3;
    
    if (has_data)
    {
      gridfunc->Reset();
      gridpoints->Initialize();
      gridvalues->Initialize();
    }
    
    for (int icell=0;icell<cells.size(); icell+=4)
    {
      vtkIdType 	 c[4]={cells[icell+0],cells[icell+1],cells[icell+2],cells[icell+3]};
      gridfunc->InsertNextCell(VTK_TETRA,4,c);
    }
    
    for (int ipoint=0;ipoint<points.size(); ipoint+=3)
    {
      gridpoints->InsertNextPoint(points[ipoint+0],points[ipoint+1],points[ipoint+2]);
    }
    
    
    gridvalues->SetNumberOfComponents(1);
    gridvalues->SetNumberOfTuples(npoints);
    
    grid_changed=1;
    has_data=true;
  }
  
  template <class V>
    inline
  void TetContour3D::UpdateValues(const V& values)
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
