#ifndef VTKFIG_UNSTRUCTURED_GRID_DATA_H
#define VTKFIG_UNSTRUCTURED_GRID_DATA_H

#include <memory>

#include "vtkSmartPointer.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridWriter.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkCellType.h"

namespace vtkfig
{
  
  class UnstructuredGridData
  {
  public:    
    
    vtkSmartPointer<vtkUnstructuredGrid> griddata=NULL;
    int spacedim=0;

    static std::shared_ptr<UnstructuredGridData> New() { return std::make_shared<UnstructuredGridData>(); }
    
    void WriteVTK(std::string fname)
    {
      auto writer=vtkSmartPointer<vtkUnstructuredGridWriter>::New();
      writer->SetFileTypeToBinary();
      writer->SetFileName(fname.c_str());
      writer->SetInputData(griddata);
      writer->Write();
    }

    
    template <class V, class IV>
      void SetSimplexVolumeGrid(int dim,
                                const V& points, 
                                const IV& cells)
    {
      if (griddata==NULL)
        griddata=vtkSmartPointer<vtkUnstructuredGrid>::New();
      else
        griddata->Reset();

      spacedim=dim;
      auto gridpoints = vtkSmartPointer<vtkPoints>::New();
      griddata->SetPoints(gridpoints);
      
      if (this->spacedim==2)
      {
        for (int icell=0;icell<cells.size(); icell+=3)
        {
          vtkIdType 	 c[3]={cells[icell+0],cells[icell+1],cells[icell+2]};
          griddata->InsertNextCell(VTK_TRIANGLE,3,c);
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
          griddata->InsertNextCell(VTK_TETRA,4,c);
        }
        
        for (int ipoint=0;ipoint<points.size(); ipoint+=3)
        {
          gridpoints->InsertNextPoint(points[ipoint+0],points[ipoint+1],points[ipoint+2]);
        }
      }    
    }
    
    template <class V>
    void SetPointScalar(const V&values, const std::string name)
    {
      assert(griddata!=NULL);
      auto gridpoints=griddata->GetPoints();
      int npoints=gridpoints->GetNumberOfPoints();
      assert(npoints==values.size());
      vtkSmartPointer<vtkFloatArray>gridvalues;
      
      if  (griddata->GetPointData()->HasArray(name.c_str()))
      {
        gridvalues=vtkFloatArray::SafeDownCast(griddata->GetPointData()->GetAbstractArray(name.c_str()));
      }
      else
      {
        gridvalues=vtkSmartPointer<vtkFloatArray>::New();
        gridvalues->SetNumberOfComponents(1);
        gridvalues->SetNumberOfTuples(npoints);
        gridvalues->SetName(name.c_str());
        griddata->GetPointData()->AddArray(gridvalues);
      }
      for (int i=0;i<npoints; i++)
      {
        double v=values[i];
        gridvalues->InsertComponent(i,0,v);
      }
      gridvalues->Modified();
    }    

    template <class V>
      void SetPointVector(const V&u, const V& v, const std::string name)
    {
      assert(spacedim==2);
      assert(griddata!=NULL);
      int npoints=griddata->GetPoints()->GetNumberOfPoints();
      assert(npoints==u.size());
      assert(npoints==v.size());
      vtkSmartPointer<vtkFloatArray>gridvalues;
      
      if  (griddata->GetPointData()->HasArray(name.c_str()))
      {
        gridvalues=vtkFloatArray::SafeDownCast(griddata->GetPointData()->GetAbstractArray(name.c_str()));
      }
      else
      {
        gridvalues=vtkSmartPointer<vtkFloatArray>::New();
        gridvalues->SetNumberOfComponents(3);
        gridvalues->SetNumberOfTuples(npoints);
        gridvalues->SetName(name.c_str());
        griddata->GetPointData()->AddArray(gridvalues);
      }
      
      for (int i=0;i<npoints; i++)
      {
        gridvalues->InsertTuple3(i,u[i],v[i],0);
      }
      gridvalues->Modified();
    }    

    template <class V>
      void SetPointVector(const V&u, const V& v, const V& w, const std::string name)
    {
      assert(spacedim==2);
      assert(griddata!=NULL);
      int npoints=griddata->GetPoints()->GetNumberOfPoints();
      assert(npoints==u.size());
      assert(npoints==v.size());
      assert(npoints==w.size());
      vtkSmartPointer<vtkFloatArray>gridvalues;
      
      if  (griddata->GetPointData()->HasArray(name.c_str()))
      {
        gridvalues=vtkFloatArray::SafeDownCast(griddata->GetPointData()->GetAbstractArray(name.c_str()));
      }
      else
      {
        gridvalues=vtkSmartPointer<vtkFloatArray>::New();
        gridvalues->SetNumberOfComponents(3);
        gridvalues->SetNumberOfTuples(npoints);
        gridvalues->SetName(name.c_str());
        griddata->GetPointData()->AddArray(gridvalues);
      }
      
      for (int i=0;i<npoints; i++)
      {
        gridvalues->InsertTuple3(i,u[i],v[i],w[i]);
      }
      gridvalues->Modified();
    }    
    
  };    
}
#endif
