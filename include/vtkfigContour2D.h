#ifndef VTKFIG_CONTOUR2D_H
#define VTKFIG_CONTOUR2D_H

#include "vtkRectilinearGrid.h"
#include "vtkFloatArray.h"
#include "vtkContourFilter.h"
#include "vtkSliderWidget.h"

#include "vtkfigFigure.h"
#include "vtkfigTools.h"


namespace vtkfig
{


  ///////////////////////////////////////////
  class Contour2D: public Figure
  {
friend class mySliderCallback;
  public:
  
  Contour2D();

  static std::shared_ptr<Contour2D> New() { return std::make_shared<Contour2D>(); }
  
  template<typename V>
      void SetGrid(const V &xcoord, 
                   const V &ycoord);

  template<typename V>
    void UpdateValues(const V &values);

      
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

      void SetInteractor(vtkSmartPointer<vtkRenderWindowInteractor>);
      virtual void Build();
//      virtual void Update();
      
      double vmax=0;
      double vmin=0;
      int ncont=10;
      vtkSmartPointer<vtkContourFilter> isocontours;
      
    private:
      vtkSmartPointer<vtkFloatArray> xcoord;
      vtkSmartPointer<vtkFloatArray> ycoord;
      vtkSmartPointer<vtkFloatArray> values;
      vtkSmartPointer<vtkSliderWidget> sliderWidget;

      vtkSmartPointer<vtkLookupTable> surface_lut;
      vtkSmartPointer<vtkLookupTable> contour_lut;

      unsigned int Nx;
      unsigned int Ny;

      bool show_surface=true;
      bool show_contour=true;
      bool show_slider=true;
      bool show_surface_colorbar=true;
      bool show_contour_colorbar=false;
  };


  template<typename V>
  inline
  void Contour2D::SetGrid(const V &x, const V &y)
  {
    Nx = x.size();
    Ny = y.size();
    
    xcoord = vtkSmartPointer<vtkFloatArray>::New();
    xcoord->SetNumberOfComponents(1);
    xcoord->SetNumberOfTuples(Nx);
    
    ycoord =vtkSmartPointer<vtkFloatArray>::New();
    ycoord->SetNumberOfComponents(1);
    ycoord->SetNumberOfTuples(Ny);
    
    for (int i=0; i<Nx; i++)
      xcoord->InsertComponent(i, 0, x[i]);
    for (int i=0; i<Ny; i++)
      ycoord->InsertComponent(i, 0, y[i]);
    
    values = vtkSmartPointer<vtkFloatArray>::New();
    values->SetNumberOfComponents(1);
    values->SetNumberOfTuples(Nx*Ny);
    
    for (int j = 0, k=0; j < Ny; j++)
      for (int i = 0; i < Nx; i++)
        values->InsertComponent(k++, 0,0);
  }

  template<typename V>
  inline
  void Contour2D::UpdateValues(const V&z)
  {
    vmax=-1.0e100;
    vmin=1.0e100;

    
    for (int j = 0, k=0; j < Ny; j++)
      for (int i = 0; i < Nx; i++)
      {
        double v=z[j*Nx+i];
        vmin=std::min(v,vmin);
        vmax=std::max(v,vmax);
        values->InsertComponent(k++, 0, v);
      }

    values->Modified();
    surface_lut->SetTableRange(vmin,vmax);
    surface_lut->Modified();
    contour_lut->SetTableRange(vmin,vmax);
    contour_lut->Modified();


    double tempdiff = (vmax-vmin)/(10*ncont);
    isocontours->GenerateValues(ncont, vmin+tempdiff, vmax-tempdiff);
    
  }

}
#endif

