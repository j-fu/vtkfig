#ifndef VTKFIG_UNSTRUCTURED_GRID_DATA_H
#define VTKFIG_UNSTRUCTURED_GRID_DATA_H

#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridWriter.h"
#include "vtkFloatArray.h"

namespace vtkfig
{
  
  class UnstructuredGridData
  {

    friend class SimplexContour2;
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
      int npoints=points.size()/spacedim;
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
    void SetPointScalar(const V&values, std::string name)
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
  };    
}
#endif
