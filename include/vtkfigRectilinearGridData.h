#ifndef VTKFIG_RECTILINEAR_GRID_DATA_H
#define VTKFIG_RECTILINEAR_GRID_DATA_H

#include "vtkRectilinearGrid.h"
#include "vtkRectilinearGridWriter.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"

namespace vtkfig
{
  
  class RectilinearGridData
  {
    
  public:    
    
    vtkSmartPointer<vtkRectilinearGrid> griddata=NULL;
    int spacedim=0;
    
    static std::shared_ptr<RectilinearGridData> New() { return std::make_shared<RectilinearGridData>(); }
    
    void WriteVTK(std::string fname)
    {
      auto writer=vtkSmartPointer<vtkRectilinearGridWriter>::New();
      writer->SetFileTypeToBinary();
      writer->SetFileName(fname.c_str());
      writer->SetInputData(griddata);
      writer->Write();
    }
    
    template<typename V>
      inline
      void SetGrid(const V &x, const V &y)
    {
      vtkSmartPointer<vtkFloatArray> xcoord;
      vtkSmartPointer<vtkFloatArray> ycoord;
      assert(spacedim!=3);
      spacedim=2;
      
      if (griddata==NULL)
      {
        griddata=vtkSmartPointer<vtkRectilinearGrid>::New();
        xcoord = vtkSmartPointer<vtkFloatArray>::New();
        ycoord = vtkSmartPointer<vtkFloatArray>::New();
        griddata->SetXCoordinates(xcoord);
        griddata->SetYCoordinates(ycoord);
      }
      else
      {
        xcoord=vtkFloatArray::SafeDownCast(griddata->GetXCoordinates());
        ycoord=vtkFloatArray::SafeDownCast(griddata->GetYCoordinates());
        xcoord->Initialize();
        ycoord->Initialize();
      }
      int Nx = x.size();
      int Ny = y.size();
      
      xcoord->SetNumberOfComponents(1);
      xcoord->SetNumberOfTuples(Nx);
      
      ycoord->SetNumberOfComponents(1);
      ycoord->SetNumberOfTuples(Ny);
      
      for (int i=0; i<Nx; i++)
        xcoord->InsertComponent(i, 0, x[i]);
      for (int i=0; i<Ny; i++)
        ycoord->InsertComponent(i, 0, y[i]);
      
      griddata->SetDimensions(Nx, Ny, 1);
      griddata->Modified();
    }
    
    
    template<typename V>
      inline
      void SetGrid(const V &x, const V &y, const V &z)
    {
      vtkSmartPointer<vtkFloatArray> xcoord;
      vtkSmartPointer<vtkFloatArray> ycoord;
      vtkSmartPointer<vtkFloatArray> zcoord;
      assert(spacedim!=2);
      spacedim=3;
      
      if (griddata==NULL)
      {
        griddata=vtkSmartPointer<vtkRectilinearGrid>::New();
        xcoord = vtkSmartPointer<vtkFloatArray>::New();
        ycoord = vtkSmartPointer<vtkFloatArray>::New();
        zcoord = vtkSmartPointer<vtkFloatArray>::New();
        griddata->SetXCoordinates(xcoord);
        griddata->SetYCoordinates(ycoord);
        griddata->SetZCoordinates(zcoord);
      }
      else
      {
        xcoord=vtkFloatArray::SafeDownCast(griddata->GetXCoordinates());
        ycoord=vtkFloatArray::SafeDownCast(griddata->GetYCoordinates());
        zcoord=vtkFloatArray::SafeDownCast(griddata->GetZCoordinates());
        xcoord->Initialize();
        ycoord->Initialize();
        zcoord->Initialize();
      }
      int Nx = x.size();
      int Ny = y.size();
      int Nz = z.size();
      
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
      
      griddata->SetDimensions(Nx, Ny, Nz );
      griddata->Modified();
    }
    
    
    
    template <class V>
    void SetPointScalar(const V&values, std::string name)
    {
      assert(griddata!=NULL);
      int dimensions[3];
      griddata->GetDimensions(dimensions);
      int npoints=dimensions[0]*dimensions[1]*dimensions[2];
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
