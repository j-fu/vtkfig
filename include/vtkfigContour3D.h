#ifndef VTKFIG_CONTOUR3D_H
#define VTKFIG_CONTOUR3D_H

#include "vtkRectilinearGrid.h"
#include "vtkPointData.h"
#include "vtkFloatArray.h"

#include "vtkfigFigure.h"

namespace vtkfig
{






///////////////////////////////////////////
class Contour3D: public Figure
    {
    public:
      
      Contour3D();
      
      template<typename V>
      void Add(const V &xcoord, 
               const V &ycoord, 
               const V &zcoord, 
               const V &values);

      
      void ShowSlice(bool b) {show_slice=b;}
      void ShowContour(bool b) {show_contour=b;}
      void ShowSliceColorbar(bool b) {show_slice_colorbar=b;}
      void ShowContourColorbar(bool b) {show_contour_colorbar=b;}

      void SetSliceRGBTable(RGBTable & tab, int tabsize)
      {
        slice_lut=BuildLookupTable(tab,tabsize);
      }
      void SetContourRGBTable(RGBTable & tab, int tabsize)
      {
        contour_lut=BuildLookupTable(tab,tabsize);
      }
      
    private:

      void Add(const vtkSmartPointer<vtkFloatArray> xcoord,
               const vtkSmartPointer<vtkFloatArray> ycoord,
               const vtkSmartPointer<vtkFloatArray> zcoord,
               const vtkSmartPointer<vtkFloatArray> values);

      vtkSmartPointer<vtkLookupTable> slice_lut;
      vtkSmartPointer<vtkLookupTable> contour_lut;

      bool show_slice=true;
      bool show_contour=true;
      bool show_slice_colorbar=true;
      bool show_contour_colorbar=true;
  };

  template<typename V>
  inline
  void Contour3D::Add(const V &x, const V &y, const V &z, const V &v)
  {
    const unsigned int Nx = x.size();
    const unsigned int Ny = y.size();
    const unsigned int Nz = z.size();
    assert(v.size()==(Nx*Ny*Nz));


    vtkSmartPointer<vtkFloatArray> xcoord = vtkSmartPointer<vtkFloatArray>::New();
    xcoord->SetNumberOfComponents(1);
    xcoord->SetNumberOfTuples(Nx);
  
    vtkSmartPointer<vtkFloatArray> ycoord =vtkSmartPointer<vtkFloatArray>::New();
    ycoord->SetNumberOfComponents(1);
    ycoord->SetNumberOfTuples(Ny);
  
    vtkSmartPointer<vtkFloatArray> zcoord =vtkSmartPointer<vtkFloatArray>::New();
    zcoord->SetNumberOfComponents(1);
    zcoord->SetNumberOfTuples(Nz);
  
    for (int i=0; i<Nx; i++)
      xcoord->InsertComponent(i, 0, x[i]);
    for (int i=0; i<Ny; i++)
      ycoord->InsertComponent(i, 0, y[i]);
    for (int i=0; i<Nz; i++)
      zcoord->InsertComponent(i, 0, z[i]);
  
    vtkSmartPointer<vtkFloatArray>values = vtkSmartPointer<vtkFloatArray>::New();
    values->SetNumberOfComponents(1);
    values->SetNumberOfTuples(Nx*Ny*Nz);
  
  
    for (int k = 0, l=0; k < Nz; k++)
      for (int j = 0; j < Ny; j++)
        for (int i = 0; i < Nx; i++)
          values->InsertComponent(l++, 0, v[k*Nx*Ny+j*Nx+i]);
  
    Add(xcoord,ycoord, zcoord, values);
  }



}

#endif
