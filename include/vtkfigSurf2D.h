#ifndef VTKFIG_SURF2D_H
#define VTKFIG_SURF2D_H

#include "vtkRectilinearGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkPointData.h"
#include "vtkFloatArray.h"


#include "vtkfigFigure.h"
#include "vtkfigTools.h"

namespace vtkfig
{

  ///////////////////////////////////////////
  class Surf2D: public Figure
    {
    public:
      
      Surf2D();
      

      template<typename V>
      void Add(const V &xcoord, 
               const V &ycoord, 
               const V &values);

      void SetRGBTable(RGBTable & tab, int tabsize)
      {
        lut=BuildLookupTable(tab,tabsize);
      }
      void ShowColorbar(bool b) {show_colorbar=b;}

    private:
      void Add(vtkSmartPointer<vtkStructuredGrid> gridfunc, double Lxy, double Lz);
      vtkSmartPointer<vtkLookupTable> lut;
      bool show_colorbar=true;
  };


  template<typename V>
  inline
  void Surf2D::Add(const V &x, const V &y, const V &z)
  {
    const unsigned int Nx = x.size();
    const unsigned int Ny = y.size();
    int i,j,k;
    double Lxy,Lz;
    
    if (x[Nx-1]-x[0] > y[Ny-1]-y[0])
      Lxy = x[Nx-1]-x[0];
    else
      Lxy = y[Ny-1]-y[0];
    double z_low = 10000, z_upp = -10000;
    
    
    vtkSmartPointer<vtkStructuredGrid> 	    gridfunc= vtkSmartPointer<vtkStructuredGrid>::New();
    gridfunc->SetDimensions(Nx, Ny, 1);
    
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    for (j = 0; j < Ny; j++)
    {
      for (i = 0; i < Nx; i++)
      {
        points->InsertNextPoint(x[i], y[j], z[j*Nx+i]);
        
        if (z[j*Nx+i]< z_low)
          z_low = z[j*Nx+i];
        if (z[j*Nx+i]> z_upp)
          z_upp = z[j*Nx+i];
      }
    }
    gridfunc->SetPoints(points);
    
    
    vtkSmartPointer<vtkFloatArray> colors = vtkSmartPointer<vtkFloatArray>::New();
    colors->SetNumberOfComponents(1);
    colors->SetNumberOfTuples(Nx*Ny);
    k = 0;
    for (j = 0; j < Ny; j++)
      for (i = 0; i < Nx; i++)
      {
        colors->InsertComponent(k, 0, z[j*Nx+i]);
        k++;
      }
    
    gridfunc->GetPointData()->SetScalars(colors);
    
    Lz = z_upp-z_low;
    
    Add(gridfunc,Lxy,Lz);
    
  }

}

#endif
