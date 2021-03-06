/**
    \file vtkfigDataSet.h

    Provide user API class vtkfig::DataSet  intended to hold all data to be rendered.
*/

#ifndef VTKFIG_DATASET_H
#define VTKFIG_DATASET_H

#include <memory>
#include <map>
#include <cmath>

#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkRectilinearGrid.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkIdList.h>


namespace vtkfig
{
  
  ///
  /// Class to collect all data given on one grid.
  /// 
  /// Wrapper of [vtkRectilinearGrid](http://www.vtk.org/doc/nightly/html/classvtkRectilinearGrid.html)
  /// resp. [vtkUnstructuredGrid](http://www.vtk.org/doc/nightly/html/classvtkUnstructuredGrid.html)
  /// Using [duck typing](https://en.wikipedia.org/wiki/Duck_typing),
  /// this class internally builds copies of the data in the form
  /// of a [vtkDataSet](http://www.vtk.org/doc/nightly/html/classvtkDataSet.html)
  ///

  class DataSet
  {

  public:    


    /// Static constructor of smart pointer to an empty instance
    static std::shared_ptr<DataSet> New();


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
    /// Set boundary cell region data (for multiregion grids)
    ///
    /// \tparam V Vector class counting from zero with member functions
    ///  size() and operator[]. std::vector will work.
    ///
    /// \param cr Cell region numbers
    ///
    /// \param name Name of function
    template <class V>
      void SetBoundaryCellRegions(const V& cr);
    

    ///
    /// Set boundary cells for simplex grids
    ///
    /// \tparam IV Vector class counting from zero with member functions
    ///  size() and operator[]. std::vector will work.
    ///
    /// \param cr Cell region numbers
    ///
    /// \param name Name of function
    template <class IV>
      void SetSimplexGridBoundaryCells( const IV& cells);
    


    /// 
    /// Mask cells which shall be shown
    ///
    /// \tparam IV Vector class counting from zero with member functions
    ///  size() and operator[]. std::vector will work.
    /// \param mask_in Flag array indicating if cell is in 
    /// \param name Name of the mask for later reference
    ///
    template <class IV>
      void SetCellMaskByMask(const  IV& mask_in, const std::string name);

    /// 
    /// Mask cells which shall be shown
    ///
    /// \tparam IV Vector class counting from zero with member functions
    ///  size() and operator[]. std::vector will work.
    /// \param cells_in List of cells which shall be shown
    /// \param name Name of the mask for later reference
    ///
    template <class IV>
      void SetCellMaskByList(const  IV& cells_in, const std::string name);

    

    /// 
    /// Mask cells which shall be shown.
    ///
    /// Assumes that DataSet::SetCellRegions() was called.
    ///
    /// \tparam IV Vector class counting from zero with member functions
    ///  size() and operator[]. std::vector will work.
    /// \param regions_omitted List of regions which shall be omitted
    /// \param name Name of the mask for later reference
    ///
    template <class IV>
      void SetCellMaskByRegionsOmitted(const  IV& regions_omitted, const std::string name);


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
    /// Set data of a vector function defined on the points on a 2D grid
    ///
    /// \tparam V Vector class counting from zero with member functions
    ///  size() and operator[]. std::vector will work.
    ///
    /// \param uvw  Vector of row-wise stored component values ( u1 v1, w1, u2,v2,w2 ...)
    ///
    /// \param name Name of function
    template <class V>
    void SetPointVector(const V&uvw, const int dim, const std::string name);


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
    /// Set data of a scalar function defined on the cells of the grid
    ///
    /// \tparam V Vector class counting from zero with member functions
    ///  size() and operator[]. std::vector will work.
    ///
    /// \param f  Vector of function values
    ///
    /// \param name Name of function
    template <class V>
      void SetBoundaryCellScalar(const V&f, const std::string name);

    ///
    /// Set scale factor for coordinates. 
    ///
    /// This has to be set before the grid is added to the dataset.
    /// Some vtk algorithms (notably vtkProbeFilter) struggle with small scale 
    /// grids, and so one can try to increase the magnitude of coordinates.
    ///
    void SetCoordinateScaleFactor(double factor) { coordinate_scale_factor=factor;}

    ///
    /// Set scale factor for x coordinates 
    ///
    /// This has to be set before the grid is added to the dataset.
    ///
    void SetCoordinateScaleFactorX(double factor) { coordinate_scale_factor_xyz[0]=factor;}

    ///
    /// Set scale factor for y coordinates 
    ///
    /// This has to be set before the grid is added to the dataset.
    ///
    void SetCoordinateScaleFactorY(double factor) { coordinate_scale_factor_xyz[1]=factor;}

    ///
    /// Set scale factor for z coordinates 
    ///
    /// This has to be set before the grid is added to the dataset.
    ///
    void SetCoordinateScaleFactorZ(double factor) { coordinate_scale_factor_xyz[2]=factor;}


    ///
    /// Write dataset to disk in VTK format
    ///
    /// \param fname File name
    /// \param filetype  File type: "A" for ASCII, "B" for binary
    ///
    void WriteVTK(const std::string fname, const std::string filetype);
    
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
    
    /// 
    /// Request boundary dataset.
    /// 
    /// \return vtkDataset
    vtkSmartPointer<vtkDataSet> GetVTKBoundaryDataSet() { return boundary_data;}
    

    /// 
    /// Request celllist
    /// 
    /// \return Cell List
    vtkSmartPointer<vtkIdList> GetCellList(std::string name) {return masks[name];}
  

  protected:
    int spacedim=0;

    DataSet();
    ~DataSet(){};

  private:

    friend class Figure;
    vtkSmartPointer<vtkDataSet> data=NULL;
    vtkSmartPointer<vtkDataSet> boundary_data=NULL;
    double coordinate_scale_factor=1.0;
    double coordinate_scale_factor_xyz[3]={1.0,1.0,1.0};

    
    template<class DATA, class WRITER>
      void WriteVTK(vtkSmartPointer<DATA> data, const std::string fname, const std::string filetype);

    std::map<std::string,vtkSmartPointer<vtkIdList>> masks;

    
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
      for (size_t icell=0;icell<cells.size(); icell+=3)
      {
        vtkIdType 	 c[3]={cells[icell+0],cells[icell+1],cells[icell+2]};
        udata->InsertNextCell(VTK_TRIANGLE,3,c);
      }
      
      for (size_t ipoint=0;ipoint<points.size(); ipoint+=2)
      {
        gridpoints->InsertNextPoint(
          points[ipoint+0]*coordinate_scale_factor*coordinate_scale_factor_xyz[0],
          points[ipoint+1]*coordinate_scale_factor*coordinate_scale_factor_xyz[1],
          0);
      }
    }
    else
    {
      for (size_t icell=0;icell<cells.size(); icell+=4)
      {
        vtkIdType 	 c[4]={cells[icell+0],cells[icell+1],cells[icell+2],cells[icell+3]};
        udata->InsertNextCell(VTK_TETRA,4,c);
      }
      
      for (size_t ipoint=0;ipoint<points.size(); ipoint+=3)
      {
        gridpoints->InsertNextPoint(points[ipoint+0]*coordinate_scale_factor*coordinate_scale_factor_xyz[0],
				    points[ipoint+1]*coordinate_scale_factor*coordinate_scale_factor_xyz[1],
                                    points[ipoint+2]*coordinate_scale_factor*coordinate_scale_factor_xyz[2]
	  );
      }
    }    
  }
  
  template <class IV>
    inline
    void DataSet::SetSimplexGridBoundaryCells(const IV& cells)
  {
    if (this->boundary_data==NULL)
      this->boundary_data=vtkSmartPointer<vtkUnstructuredGrid>::New();

    auto budata=vtkUnstructuredGrid::SafeDownCast(this->boundary_data);
    budata->Reset();

    auto bgridpoints = vtkSmartPointer<vtkPoints>::New();
    budata->SetPoints(bgridpoints);
    
    auto udata=vtkUnstructuredGrid::SafeDownCast(this->data);
    auto pdata=udata->GetPoints();
    
    int np=pdata->GetNumberOfPoints();
    std::vector<int>pmask(np);
    for (size_t i=0;i<pmask.size(); i++) 
    {
      pmask[i]=-1;
    }



    int ip=0;
    for (int icell=0;icell<cells.size();icell+=this->spacedim)
    {
      for (int id=0;id<this->spacedim;id++)
      {
        if (pmask[cells[icell+id]]==-1)
        {
          pmask[cells[icell+id]]=ip++;
          double point[3];
          pdata->GetPoint(cells[icell+id],point);
          bgridpoints->InsertNextPoint(point[0],point[1],point[2]);
        }

      }
      if (this->spacedim==2)
      {
        vtkIdType 	 c[2]={pmask[cells[icell+0]],pmask[cells[icell+1]]};
        budata->InsertNextCell(VTK_LINE,2,c);
      }
      else
      {
        vtkIdType 	 c[3]={pmask[cells[icell+0]],pmask[cells[icell+1]],pmask[cells[icell+2]]};
        budata->InsertNextCell(VTK_TRIANGLE,3,c);
      }

    }

  }
  


    
  template<typename V>
    inline
    void DataSet::SetRectilinearGrid(const V &x, const V &y)
  {
    vtkSmartPointer<vtkDoubleArray> xcoord;
    vtkSmartPointer<vtkDoubleArray> ycoord;
    assert(spacedim!=3);
    spacedim=2;
    int Nx = x.size();
    int Ny = y.size();
    
    if (this->data==NULL)
    {
      this->data=vtkSmartPointer<vtkRectilinearGrid>::New();
      auto rdata=vtkRectilinearGrid::SafeDownCast(this->data);
      assert(rdata);
      xcoord = vtkSmartPointer<vtkDoubleArray>::New();
      ycoord = vtkSmartPointer<vtkDoubleArray>::New();
      rdata->SetXCoordinates(xcoord);
      rdata->SetYCoordinates(ycoord);
      rdata->SetDimensions(Nx, Ny, 1);
    }
    else
    {
      auto rdata=vtkRectilinearGrid::SafeDownCast(this->data);
      assert(rdata);
      xcoord=vtkDoubleArray::SafeDownCast(rdata->GetXCoordinates());
      ycoord=vtkDoubleArray::SafeDownCast(rdata->GetYCoordinates());
      xcoord->Initialize();
      ycoord->Initialize();
      rdata->SetDimensions(Nx, Ny, 1);
    }
    
    xcoord->SetNumberOfComponents(1);
    xcoord->SetNumberOfTuples(Nx);
    
    ycoord->SetNumberOfComponents(1);
    ycoord->SetNumberOfTuples(Ny);
    
    for (int i=0; i<Nx; i++)
      xcoord->InsertComponent(i, 0, x[i]*coordinate_scale_factor*coordinate_scale_factor_xyz[0]);
    for (int i=0; i<Ny; i++)
      ycoord->InsertComponent(i, 0, y[i]*coordinate_scale_factor*coordinate_scale_factor_xyz[1]);
    
    this->data->Modified();
  }
  
  
  template<typename V>
    inline
    void DataSet::SetRectilinearGrid(const V &x, const V &y, const V &z)
  {
    vtkSmartPointer<vtkDoubleArray> xcoord;
    vtkSmartPointer<vtkDoubleArray> ycoord;
    vtkSmartPointer<vtkDoubleArray> zcoord;
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
      xcoord = vtkSmartPointer<vtkDoubleArray>::New();
      ycoord = vtkSmartPointer<vtkDoubleArray>::New();
      zcoord = vtkSmartPointer<vtkDoubleArray>::New();
      rdata->SetXCoordinates(xcoord);
      rdata->SetYCoordinates(ycoord);
      rdata->SetZCoordinates(zcoord);
      rdata->SetDimensions(Nx, Ny, Nz );
    }
    else
    {
      auto rdata=vtkRectilinearGrid::SafeDownCast(this->data);
      assert(rdata);
      xcoord=vtkDoubleArray::SafeDownCast(rdata->GetXCoordinates());
      ycoord=vtkDoubleArray::SafeDownCast(rdata->GetYCoordinates());
      zcoord=vtkDoubleArray::SafeDownCast(rdata->GetZCoordinates());
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
      xcoord->InsertComponent(i, 0, x[i]*coordinate_scale_factor*coordinate_scale_factor_xyz[0]);
    for (int i=0; i<Ny; i++)
      ycoord->InsertComponent(i, 0, y[i]*coordinate_scale_factor*coordinate_scale_factor_xyz[1]);
    for (int i=0; i<Nz; i++)
      zcoord->InsertComponent(i, 0, z[i]*coordinate_scale_factor*coordinate_scale_factor_xyz[2]);
    
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
    void DataSet::SetBoundaryCellRegions(const V&values)
  {
    SetBoundaryCellScalar(values, "boundarycellregions");
  }
  
  
  template <class V>
    inline 
    void DataSet::SetCellScalar(const V&values, const std::string name)
  {
    assert(this->data!=NULL);
    auto ncells=this->data->GetNumberOfCells();
    assert(ncells==values.size());
    vtkSmartPointer<vtkDoubleArray>gridvalues;
    
    if  (this->data->GetPointData()->HasArray(name.c_str()))
      gridvalues=vtkDoubleArray::SafeDownCast(this->data->GetPointData()->GetAbstractArray(name.c_str()));
    else
    {
      gridvalues=vtkSmartPointer<vtkDoubleArray>::New();
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
    void DataSet::SetBoundaryCellScalar(const V&values, const std::string name)
  {
    assert(this->boundary_data!=NULL);
    auto ncells=this->boundary_data->GetNumberOfCells();
    assert(ncells==values.size());
    vtkSmartPointer<vtkDoubleArray>gridvalues;
    
    if  (this->boundary_data->GetPointData()->HasArray(name.c_str()))
      gridvalues=vtkDoubleArray::SafeDownCast(this->boundary_data->GetPointData()->GetAbstractArray(name.c_str()));
    else
    {
      gridvalues=vtkSmartPointer<vtkDoubleArray>::New();
      gridvalues->SetNumberOfComponents(1);
      gridvalues->SetNumberOfTuples(ncells);
      gridvalues->SetName(name.c_str());
      this->boundary_data->GetCellData()->AddArray(gridvalues);
    }


    for (int i=0;i<ncells; i++)
      gridvalues->InsertComponent(i,0,values[i]);
    
    gridvalues->Modified();
  }    
  
  
  
  template <class IV>
    inline
    void DataSet::SetCellMaskByRegionsOmitted(const  IV& regions_omitted, const std::string name)
  {
    auto celllist=vtkSmartPointer<vtkIdList>::New();
    auto cellregions=vtkDoubleArray::SafeDownCast(this->data->GetCellData()->GetAbstractArray("cellregions"));
    assert(cellregions);
    int ncells=this->data->GetNumberOfCells();
    int icelllist=0;
    for (int icell=0;icell<ncells;icell++)
    {
      int ireg=cellregions->GetComponent(icell,0);
      bool use_cell=true;
      
      for (int io=0;io<regions_omitted.size();io++)
        if (ireg==regions_omitted[io])
        {
          use_cell=false;
          continue;
        }
      if (use_cell)
        celllist->InsertId(icelllist++,icell);
    }
    masks[name]=celllist;
  }
  
    
    
  template <class IV>
    inline
    void DataSet::SetCellMaskByMask(const  IV& cells_in, const std::string name)
  {
    assert(cells_in.size()==this->data->GetNumberOfCells());
    auto celllist=vtkSmartPointer<vtkIdList>::New();
    int j=0;
    for (int i=0;i<cells_in.size();i++)
      if (cells_in[i]) celllist->InsertId(j++,i);
    masks[name]=celllist;
  }
  
  template <class IV>
    inline
    void DataSet::SetCellMaskByList(const  IV& cells_in, const std::string name)
  {
    auto celllist=vtkSmartPointer<vtkIdList>::New();
    int j=0;
    for (int i=0;i<cells_in.size();i++)
      celllist->InsertId(j++,cells_in[i]);
    masks[name]=celllist;
  }
  
    
  
  
  template <class V>
    inline
    void DataSet::SetPointScalar(const V&values, const std::string name)
  {
    assert(this->data!=NULL);
    size_t npoints=this->data->GetNumberOfPoints();
    assert(npoints==values.size());
    vtkSmartPointer<vtkDoubleArray>gridvalues;
    
    if  (this->data->GetPointData()->HasArray(name.c_str()))
      gridvalues=vtkDoubleArray::SafeDownCast(this->data->GetPointData()->GetAbstractArray(name.c_str()));
    else
    {
      gridvalues=vtkSmartPointer<vtkDoubleArray>::New();
      gridvalues->SetNumberOfComponents(1);
      gridvalues->SetNumberOfTuples(npoints);
      gridvalues->SetName(name.c_str());
      this->data->GetPointData()->AddArray(gridvalues);
    }

    for (size_t i=0;i<npoints; i++)
      gridvalues->InsertComponent(i,0,values[i]);
    gridvalues->Modified();
  }
  
  template <class V>
    inline
    void DataSet::SetPointVector(const V&u, const V& v, const std::string name)
  {
    assert(this->spacedim==2);
    assert(this->data!=NULL);
    size_t npoints=this->data->GetNumberOfPoints();
    assert(npoints==u.size());
    assert(npoints==v.size());
    vtkSmartPointer<vtkDoubleArray>gridvalues;
    
    if  (this->data->GetPointData()->HasArray(name.c_str()))
      gridvalues=vtkDoubleArray::SafeDownCast(this->data->GetPointData()->GetAbstractArray(name.c_str()));
    else
    {
      gridvalues=vtkSmartPointer<vtkDoubleArray>::New();
      gridvalues->SetNumberOfComponents(3);
      gridvalues->SetNumberOfTuples(npoints);
      gridvalues->SetName(name.c_str());
      this->data->GetPointData()->AddArray(gridvalues);
    }
    
    for (size_t i=0;i<npoints; i++)
      gridvalues->InsertTuple3(i,u[i],v[i],0);
    
    
    gridvalues->Modified();
  }    
  
  
  
  template <class V>
    inline
    void DataSet::SetPointVector(const V&u, const V& v, const V& w, const std::string name)
  {
    assert(this->spacedim==3);
    assert(this->data!=NULL);
    size_t npoints=this->data->GetNumberOfPoints();
    assert(npoints==u.size());
    assert(npoints==v.size());
    assert(npoints==w.size());
    vtkSmartPointer<vtkDoubleArray>gridvalues;
    
    if  (this->data->GetPointData()->HasArray(name.c_str()))
      gridvalues=vtkDoubleArray::SafeDownCast(this->data->GetPointData()->GetAbstractArray(name.c_str()));
    else
    {
      gridvalues=vtkSmartPointer<vtkDoubleArray>::New();
      gridvalues->SetNumberOfComponents(3);
      gridvalues->SetNumberOfTuples(npoints);
      gridvalues->SetName(name.c_str());
      this->data->GetPointData()->AddArray(gridvalues);
    }
    

    for (size_t i=0;i<npoints; i++)
      gridvalues->InsertTuple3(i,u[i],v[i],w[i]);
    gridvalues->Modified();
  }    


  template <class V>
    inline
    void DataSet::SetPointVector(const V&uvw, int dim, const std::string name)
  {
    assert(this->spacedim==dim);
    assert(this->data!=NULL);
    size_t npoints=this->data->GetNumberOfPoints();
    assert(npoints==uvw.size()/dim);

    vtkSmartPointer<vtkDoubleArray>gridvalues;
    
    if  (this->data->GetPointData()->HasArray(name.c_str()))
    {
      gridvalues=vtkDoubleArray::SafeDownCast(this->data->GetPointData()->GetAbstractArray(name.c_str()));
    }
    else
    {
      gridvalues=vtkSmartPointer<vtkDoubleArray>::New();
      gridvalues->SetNumberOfComponents(3);
      gridvalues->SetNumberOfTuples(npoints);
      gridvalues->SetName(name.c_str());
      this->data->GetPointData()->AddArray(gridvalues);
    }
    
    switch(dim)
    {
    case 2:
      for (size_t i=0,j=0;i<npoints; i++,j+=2)
        gridvalues->InsertTuple3(i,uvw[j],uvw[j+1],0);
      break;
    case 3:
      for (size_t i=0,j=0;i<npoints; i++,j+=3)
        gridvalues->InsertTuple3(i,uvw[j],uvw[j+1],uvw[j+2]);
      break;
    default: break;
    }
    
    gridvalues->Modified();
  }    
  
}


#endif
