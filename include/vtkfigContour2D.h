#ifndef VTKFIG_CONTOUR2D_H
#define VTKFIG_CONTOUR2D_H

#include "vtkRectilinearGrid.h"
#include "vtkFloatArray.h"

#include "vtkfigFigure.h"
#include "vtkfigTools.h"


namespace vtkfig
{


  ///////////////////////////////////////////
  class Contour2D: public Figure
  {
  public:
  
  Contour2D();
  
  template<typename V>
      void Add(const V &xcoord, 
               const V &ycoord, 
               const V &values);

      
      void ShowSurface(bool b) {show_surface=b;}
      void ShowContour(bool b) {show_contour=b;}
      void ShowSurfaceColorbar(bool b) {show_surface_colorbar=b;}
      void ShowContourColorbar(bool b) {show_contour_colorbar=b;}

      void SetSurfaceRGBTable(RGBTable & tab, int tabsize)
      {
        surface_lut=BuildLookupTable(tab,tabsize);
      }
      void SetContourRGBTable(RGBTable & tab, int tabsize)
      {
        contour_lut=BuildLookupTable(tab,tabsize);
      }
      
    private:
      void Add(const vtkSmartPointer<vtkFloatArray> xcoord,
               const vtkSmartPointer<vtkFloatArray> ycoord,
               const vtkSmartPointer<vtkFloatArray> values);

      vtkSmartPointer<vtkLookupTable> surface_lut;
      vtkSmartPointer<vtkLookupTable> contour_lut;

      bool show_surface=true;
      bool show_contour=true;
      bool show_surface_colorbar=true;
      bool show_contour_colorbar=false;
  };


  template<typename V>
  inline
  void Contour2D::Add(const V &x, const V &y, const V &z)
  {
    const unsigned int Nx = x.size();
    const unsigned int Ny = y.size();
    
    vtkSmartPointer<vtkFloatArray> xcoord = vtkSmartPointer<vtkFloatArray>::New();
    xcoord->SetNumberOfComponents(1);
    xcoord->SetNumberOfTuples(Nx);
    
    vtkSmartPointer<vtkFloatArray> ycoord =vtkSmartPointer<vtkFloatArray>::New();
    ycoord->SetNumberOfComponents(1);
    ycoord->SetNumberOfTuples(Ny);
    
    for (int i=0; i<Nx; i++)
      xcoord->InsertComponent(i, 0, x[i]);
    for (int i=0; i<Ny; i++)
      ycoord->InsertComponent(i, 0, y[i]);
    
    vtkSmartPointer<vtkFloatArray>values = vtkSmartPointer<vtkFloatArray>::New();
    values->SetNumberOfComponents(1);
    values->SetNumberOfTuples(Nx*Ny);
    
    for (int j = 0, k=0; j < Ny; j++)
      for (int i = 0; i < Nx; i++)
        values->InsertComponent(k++, 0, z[j*Nx+i]);
    
    Add(xcoord,ycoord, values);
    
  }

}
#endif

