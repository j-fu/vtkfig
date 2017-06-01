#ifndef VTKFIG_DATASET_H
#define VTKFIG_DATASET_H

#include <memory>

#include "vtkSmartPointer.h"
#include "vtkUnstructuredGrid.h"
#include "vtkRectilinearGrid.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkCellType.h"

namespace vtkfig
{
  
  ///
  /// Class to collect all data given on one grid.
  /// 
  /// Wrapper of vtkRectilinearGridData resp. vtkUnstructuredGridData.
  /// Using [duck typing](https://en.wikipedia.org/wiki/Duck_typing),
  /// this class internally builds copies of the data in the form
  /// of a vtkDataSet.
  ///

  class DataSet
  {

  public:    
    
    /// Static constructor of an empty instance
    static std::shared_ptr<DataSet> New() { return std::make_shared<DataSet>(); }

    ///
    /// Enter data of a simplex grid.  
    ///
    /// \tparam V Vector class counting from zero with member functions
    ///  size() and operator[]. std::vector will work.
    ///
    /// \tparam IV Vector class counting from zero with member functions
    /// size() and operator[]. std::vector will work.
    ///
    /// \param dim  Space dimension (2 or 3)
    ///
    /// \param points  Point coordinates, stored consecutively.<br>
    ///      In 2D, coordinates of point i are `(points[2*i],points[2*i+1])`<br>
    ///      In 3D, coordinates of point i are `(points[3*i],points[3*i+1],,points[3*i+2])`
    ///
    /// \param cells  Simplex point indices (counting from 0!)
    ///      In 2D, point indices of triangle i are `(cells[3*i],cells[3*i+1],cells[3*i+2])`<br>
    ///      In 3D, point indices of tetrahedron i are `(cells[3*i],cells[3*i+1],cells[3*i+2],cells[3*i+3])`

    template <class V, class IV>
      void SetSimplexGrid(int dim, const V& points,  const IV& cells);

    ///
    /// Enter data of a 2D rectilinear grid.  
    ///
    /// \tparam V Vector class counting from zero with member functions
    ///  size() and operator[]. std::vector will work.
    ///
    /// \param x   x-coordinates
    /// \param y   y-coordinates
    template<typename V> 
      void SetRectilinearGrid(const V &x, const V &y);

    ///
    /// Enter data of a 3D rectilinear grid.  
    ///
    /// \tparam V Vector class counting from zero with member functions
    ///  size() and operator[]. std::vector will work.
    ///
    /// \param x   x-coordinates
    /// \param y   y-coordinates
    /// \param z   z-coordinates
    template<typename V>
      void SetRectilinearGrid(const V &x, const V &y, const V &z);

    ///
    /// Set data of a scalar function defined on the points of the grid
    ///
    /// \tparam V Vector class counting from zero with member functions
    ///  size() and operator[]. std::vector will work.
    ///
    /// \param f  Vector of function values
    ///
    /// \param name Name of function
    template <class V>
      void SetPointScalar(const V&f , const std::string name);

    ///
    /// Set data of a vector function defined on the points on a 2D grid
    ///
    /// \tparam V Vector class counting from zero with member functions
    ///  size() and operator[]. std::vector will work.
    ///
    /// \param u  Vector of x component values
    ///
    /// \param v  Vector of y component values
    ///
    /// \param name Name of function
    template <class V>
      void SetPointVector(const V&u, const V& v, const std::string name);

    ///
    /// Set data of a vector function defined on the points on a 2D grid
    ///
    /// \tparam V Vector class counting from zero with member functions
    ///  size() and operator[]. std::vector will work.
    ///
    /// \param u  Vector of x component values
    ///
    /// \param v  Vector of y component values
    ///
    /// \param w  Vector of z component values
    ///
    /// \param name Name of function
    template <class V>
      void SetPointVector(const V&u, const V& v, const V& w, const std::string name);

    ///
    /// Set data of a scalar function defined on the cells of the grid
    ///
    /// \tparam V Vector class counting from zero with member functions
    ///  size() and operator[]. std::vector will work.
    ///
    /// \param f  Vector of function values
    ///
    /// \param name Name of function
    template <class V>
      void SetCellScalar(const V&f, const std::string name);

    ///
    /// Set cell region data (for multiregion grids)
    ///
    /// \tparam V Vector class counting from zero with member functions
    ///  size() and operator[]. std::vector will work.
    ///
    /// \param cr Cell region numbers
    ///
    /// \param name Name of function
    template <class V>
      void SetCellRegions(const V& cr);

    ///
    /// Write dataset to disk in VTK format
    ///
    /// \param fname File name
    void WriteVTK(std::string fname);
    
    /// 
    /// Request the space dimension of the dataset
    /// 
    /// \return Space dimension
    int GetSpaceDimension() { return spacedim;}
    
    ///
    /// Enum describing different possible data types.
    ///
    enum class DataType
    {
      NoType=0,
        
        RectilinearGrid=1,
        
        UnstructuredGrid=2
        
        };

    /// 
    /// Request the data type of the dataset
    /// 
    /// \return Data type
    DataType GetDataType();

    /// 
    /// Request vtkDataset which contains all the data.
    /// 
    /// \return vtkDataset
    vtkSmartPointer<vtkDataSet> GetVTKDataSet() { return data;}
    
  private:
    
    vtkSmartPointer<vtkDataSet> data=NULL;
    int spacedim=0;
    
    template<class DATA, class WRITER>
      void WriteVTK(vtkSmartPointer<DATA> data, std::string fname);
    
  };    
  
  
  template <class V, class IV>
    inline
    void DataSet::SetSimplexGrid(int dim,
                                 const V& points, 
                                 const IV& cells)
  {
    if (this->data==NULL)
      this->data=vtkSmartPointer<vtkUnstructuredGrid>::New();
    
    auto udata=vtkUnstructuredGrid::SafeDownCast(this->data);
    udata->Reset();
    
    this->spacedim=dim;
    auto gridpoints = vtkSmartPointer<vtkPoints>::New();
    udata->SetPoints(gridpoints);
    
    if (this->spacedim==2)
    {
      for (int icell=0;icell<cells.size(); icell+=3)
      {
        vtkIdType 	 c[3]={cells[icell+0],cells[icell+1],cells[icell+2]};
        udata->InsertNextCell(VTK_TRIANGLE,3,c);
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
        udata->InsertNextCell(VTK_TETRA,4,c);
      }
      
      for (int ipoint=0;ipoint<points.size(); ipoint+=3)
      {
        gridpoints->InsertNextPoint(points[ipoint+0],points[ipoint+1],points[ipoint+2]);
      }
    }    
  }
  
  
    
  template<typename V>
    inline
    void DataSet::SetRectilinearGrid(const V &x, const V &y)
  {
    vtkSmartPointer<vtkFloatArray> xcoord;
    vtkSmartPointer<vtkFloatArray> ycoord;
    assert(spacedim!=3);
    spacedim=2;
    int Nx = x.size();
    int Ny = y.size();
    
    if (this->data==NULL)
    {
      this->data=vtkSmartPointer<vtkRectilinearGrid>::New();
      auto rdata=vtkRectilinearGrid::SafeDownCast(this->data);
      assert(rdata);
      xcoord = vtkSmartPointer<vtkFloatArray>::New();
      ycoord = vtkSmartPointer<vtkFloatArray>::New();
      rdata->SetXCoordinates(xcoord);
      rdata->SetYCoordinates(ycoord);
      rdata->SetDimensions(Nx, Ny, 1);
    }
    else
    {
      auto rdata=vtkRectilinearGrid::SafeDownCast(this->data);
      assert(rdata);
      xcoord=vtkFloatArray::SafeDownCast(rdata->GetXCoordinates());
      ycoord=vtkFloatArray::SafeDownCast(rdata->GetYCoordinates());
      xcoord->Initialize();
      ycoord->Initialize();
      rdata->SetDimensions(Nx, Ny, 1);
    }
    
    xcoord->SetNumberOfComponents(1);
    xcoord->SetNumberOfTuples(Nx);
    
    ycoord->SetNumberOfComponents(1);
    ycoord->SetNumberOfTuples(Ny);
    
    for (int i=0; i<Nx; i++)
      xcoord->InsertComponent(i, 0, x[i]);
    for (int i=0; i<Ny; i++)
      ycoord->InsertComponent(i, 0, y[i]);
    
    this->data->Modified();
  }
  
  
  template<typename V>
    inline
    void DataSet::SetRectilinearGrid(const V &x, const V &y, const V &z)
  {
    vtkSmartPointer<vtkFloatArray> xcoord;
    vtkSmartPointer<vtkFloatArray> ycoord;
    vtkSmartPointer<vtkFloatArray> zcoord;
    assert(spacedim!=2);
    spacedim=3;
    int Nx = x.size();
    int Ny = y.size();
    int Nz = z.size();
    
    if (this->data==NULL)
    {
      this->data=vtkSmartPointer<vtkRectilinearGrid>::New();
      auto rdata=vtkRectilinearGrid::SafeDownCast(this->data);
      assert(rdata);
      xcoord = vtkSmartPointer<vtkFloatArray>::New();
      ycoord = vtkSmartPointer<vtkFloatArray>::New();
      zcoord = vtkSmartPointer<vtkFloatArray>::New();
      rdata->SetXCoordinates(xcoord);
      rdata->SetYCoordinates(ycoord);
      rdata->SetZCoordinates(zcoord);
      rdata->SetDimensions(Nx, Ny, Nz );
    }
    else
    {
      auto rdata=vtkRectilinearGrid::SafeDownCast(this->data);
      assert(rdata);
      xcoord=vtkFloatArray::SafeDownCast(rdata->GetXCoordinates());
      ycoord=vtkFloatArray::SafeDownCast(rdata->GetYCoordinates());
      zcoord=vtkFloatArray::SafeDownCast(rdata->GetZCoordinates());
      xcoord->Initialize();
      ycoord->Initialize();
      zcoord->Initialize();
      rdata->SetDimensions(Nx, Ny, Nz );
    }
    
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
    
    this->data->Modified();
  }
  
  
  
  template <class V>
    inline 
    void DataSet::SetCellRegions(const V&values)
  {
    SetCellScalar(values, "cellregions");
  }
  
  
  template <class V>
    inline 
    void DataSet::SetCellScalar(const V&values, const std::string name)
  {
    assert(this->data!=NULL);
    auto ncells=this->data->GetNumberOfCells();
    assert(ncells==values.size());
    vtkSmartPointer<vtkFloatArray>gridvalues;
    
    if  (this->data->GetPointData()->HasArray(name.c_str()))
      gridvalues=vtkFloatArray::SafeDownCast(this->data->GetPointData()->GetAbstractArray(name.c_str()));
    else
    {
      gridvalues=vtkSmartPointer<vtkFloatArray>::New();
      gridvalues->SetNumberOfComponents(1);
      gridvalues->SetNumberOfTuples(ncells);
      gridvalues->SetName(name.c_str());
      this->data->GetCellData()->AddArray(gridvalues);
    }
    for (int i=0;i<ncells; i++)
      gridvalues->InsertComponent(i,0,values[i]);
    gridvalues->Modified();
  }    
    
  
  template <class V>
    inline
    void DataSet::SetPointScalar(const V&values, const std::string name)
  {
    assert(this->data!=NULL);
    int npoints=this->data->GetNumberOfPoints();
    assert(npoints==values.size());
    vtkSmartPointer<vtkFloatArray>gridvalues;
    
    if  (this->data->GetPointData()->HasArray(name.c_str()))
      gridvalues=vtkFloatArray::SafeDownCast(this->data->GetPointData()->GetAbstractArray(name.c_str()));
    else
    {
      gridvalues=vtkSmartPointer<vtkFloatArray>::New();
      gridvalues->SetNumberOfComponents(1);
      gridvalues->SetNumberOfTuples(npoints);
      gridvalues->SetName(name.c_str());
      this->data->GetPointData()->AddArray(gridvalues);
    }
    for (int i=0;i<npoints; i++)
      gridvalues->InsertComponent(i,0,values[i]);
    gridvalues->Modified();
  }
  
  template <class V>
    inline
    void DataSet::SetPointVector(const V&u, const V& v, const std::string name)
  {
    assert(this->spacedim==2);
    assert(this->data!=NULL);
    int npoints=this->data->GetNumberOfPoints();
    assert(npoints==u.size());
    assert(npoints==v.size());
    vtkSmartPointer<vtkFloatArray>gridvalues;
    
    if  (this->data->GetPointData()->HasArray(name.c_str()))
      gridvalues=vtkFloatArray::SafeDownCast(this->data->GetPointData()->GetAbstractArray(name.c_str()));
    else
    {
      gridvalues=vtkSmartPointer<vtkFloatArray>::New();
      gridvalues->SetNumberOfComponents(3);
      gridvalues->SetNumberOfTuples(npoints);
      gridvalues->SetName(name.c_str());
      this->data->GetPointData()->AddArray(gridvalues);
    }
    
    for (int i=0;i<npoints; i++)
      gridvalues->InsertTuple3(i,u[i],v[i],0);
    
    gridvalues->Modified();
  }    
  
  
  
  template <class V>
    inline
    void DataSet::SetPointVector(const V&u, const V& v, const V& w, const std::string name)
  {
    assert(this->spacedim==2);
    assert(this->data!=NULL);
    int npoints=this->data->GetNumberOfPoints();
    assert(npoints==u.size());
    assert(npoints==v.size());
    assert(npoints==w.size());
    vtkSmartPointer<vtkFloatArray>gridvalues;
    
    if  (this->data->GetPointData()->HasArray(name.c_str()))
      gridvalues=vtkFloatArray::SafeDownCast(this->data->GetPointData()->GetAbstractArray(name.c_str()));
    else
    {
      gridvalues=vtkSmartPointer<vtkFloatArray>::New();
      gridvalues->SetNumberOfComponents(3);
      gridvalues->SetNumberOfTuples(npoints);
      gridvalues->SetName(name.c_str());
      this->data->GetPointData()->AddArray(gridvalues);
    }
    
    for (int i=0;i<npoints; i++)
      gridvalues->InsertTuple3(i,u[i],v[i],w[i]);

    gridvalues->Modified();
  }    
  
}


#endif
