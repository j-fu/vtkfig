#ifndef VTKFIG_CONTOUR3D_H
#define VTKFIG_CONTOUR3D_H

#include "vtkRectilinearGrid.h"
#include "vtkFloatArray.h"
#include "vtkContourFilter.h"

#include "vtkfigFigure.h"
#include "vtkfigTools.h"

namespace vtkfig
{


///////////////////////////////////////////
class Contour3D: public Figure
    {
    public:
      
      Contour3D();
      static std::shared_ptr<Contour3D> New() { return std::make_shared<Contour3D>(); }
      
      template<typename V>
      void SetGrid(const V &xcoord, 
                   const V &ycoord, 
                   const V &zcoord);

      template<typename V>
      void UpdateValues(const V &values);

      unsigned int Nx;
      unsigned int Ny;
      unsigned int Nz;
      double vmin;
      double vmax;

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



      virtual void RTBuild();

      vtkSmartPointer<vtkFloatArray> xcoord;
      vtkSmartPointer<vtkFloatArray> ycoord;
      vtkSmartPointer<vtkFloatArray> zcoord;
      vtkSmartPointer<vtkFloatArray> values;

      vtkSmartPointer<vtkLookupTable> slice_lut;
      vtkSmartPointer<vtkLookupTable> contour_lut;
      vtkSmartPointer<vtkContourFilter> isocontours;

      bool show_slice=true;
      bool show_contour=true;
      bool show_slice_colorbar=true;
      bool show_contour_colorbar=true;
      int ncont=10;
  };

  template<typename V>
  inline
  void Contour3D::SetGrid(const V &x, const V &y, const V &z)
  {
    Nx = x.size();
    Ny = y.size();
    Nz = z.size();


    xcoord = vtkSmartPointer<vtkFloatArray>::New();
    xcoord->SetNumberOfComponents(1);
    xcoord->SetNumberOfTuples(Nx);
  
    ycoord =vtkSmartPointer<vtkFloatArray>::New();
    ycoord->SetNumberOfComponents(1);
    ycoord->SetNumberOfTuples(Ny);
  
    zcoord =vtkSmartPointer<vtkFloatArray>::New();
    zcoord->SetNumberOfComponents(1);
    zcoord->SetNumberOfTuples(Nz);
  
    for (int i=0; i<Nx; i++)
      xcoord->InsertComponent(i, 0, x[i]);
    for (int i=0; i<Ny; i++)
      ycoord->InsertComponent(i, 0, y[i]);
    for (int i=0; i<Nz; i++)
      zcoord->InsertComponent(i, 0, z[i]);
  
    values = vtkSmartPointer<vtkFloatArray>::New();
    values->SetNumberOfComponents(1);
    values->SetNumberOfTuples(Nx*Ny*Nz);
  
  
    for (int k = 0, l=0; k < Nz; k++)
      for (int j = 0; j < Ny; j++)
        for (int i = 0; i < Nx; i++)
          values->InsertComponent(l++, 0, 0);
  }

  template<typename V>
  inline
  void Contour3D::UpdateValues(const V&z)
  {
    vmax=-1.0e100;
    vmin=1.0e100;

    
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
    slice_lut->SetTableRange(vmin,vmax);
    slice_lut->Modified();
    contour_lut->SetTableRange(vmin,vmax);
    contour_lut->Modified();
    
    
    double tempdiff = (vmax-vmin)/(10*ncont);
    isocontours->GenerateValues(ncont, vmin+tempdiff, vmax-tempdiff);
    
  }


}

#endif
