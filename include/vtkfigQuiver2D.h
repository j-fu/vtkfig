#ifndef VTKFIG_QUIVER2D_H
#define VTKFIG_QUIVER2D_H

#include "vtkFloatArray.h"
#include "vtkfigFigure.h"
#include "vtkfigTools.h"

namespace vtkfig
{


  ///////////////////////////////////////////
  class Quiver2D: public Figure
    {
    public:
      
      Quiver2D();
      static std::shared_ptr<Quiver2D> New() { return std::make_shared<Quiver2D>(); }

      template<typename V>
        void UpdateValues(const V &u,
                          const V &v);
      
      template<typename V>
        void SetGrid(const V &x, 
                     const V &y);

      void SetRGBTable(RGBTable & tab, int tabsize)
      {
        lut=BuildLookupTable(tab,tabsize);
      }
      void ShowColorbar(bool b) {show_colorbar=b;}

      void SetArrowScale(double scale) {arrow_scale=scale;}

    private:

     
      virtual void RTBuild(
        vtkSmartPointer<vtkRenderWindow> window,
        vtkSmartPointer<vtkRenderWindowInteractor> interactor,
        vtkSmartPointer<vtkRenderer> renderer);


      double vmax=0;
      double vmin=0;

      vtkSmartPointer<vtkFloatArray> xcoord;
      vtkSmartPointer<vtkFloatArray> ycoord;
      vtkSmartPointer<vtkFloatArray> colors;
      vtkSmartPointer<vtkFloatArray> vectors;
  
      vtkSmartPointer<vtkLookupTable> lut;
      bool show_colorbar=true;
      double arrow_scale=0.333;
      unsigned int Nx ;
      unsigned int Ny; 
    };  
  
  template<typename V>
    inline
    void Quiver2D::SetGrid(const V &x, 
                           const V &y)
  {
    Nx = x.size();
    Ny = x.size();

    xcoord = vtkSmartPointer<vtkFloatArray>::New();
    xcoord->SetNumberOfComponents(1);
    xcoord->SetNumberOfTuples(Nx);

    ycoord = vtkSmartPointer<vtkFloatArray>::New();
    ycoord->SetNumberOfComponents(1);
    ycoord->SetNumberOfTuples(Ny);


    colors = vtkSmartPointer<vtkFloatArray>::New();
    colors->SetNumberOfComponents(1);
    colors->SetNumberOfTuples(Nx*Ny);

    vectors = vtkSmartPointer<vtkFloatArray>::New();
    vectors->SetNumberOfComponents(3);
    vectors->SetNumberOfTuples(Nx*Ny);
    
    
    for (int i=0; i<Nx; i++)
      xcoord->InsertComponent(i, 0, x[i]);
    for (int i=0; i<Ny; i++)
      ycoord->InsertComponent(i, 0, y[i]);
    

    for (int j = 0, k=0; j < Ny; j++)
      for (int i = 0; i < Nx; i++)
      {
        colors->InsertTuple1(k,0);
        vectors->InsertTuple3(k, 0, 0, 0.0);
        k++;
      }
    
  
  }


  template<typename V>
    void Quiver2D::UpdateValues(const V &u,
                      const V &v)
  {
    
    for (int j = 0, k=0; j < Ny; j++)
      for (int i = 0; i < Nx; i++)
      {

        double vv=sqrt(u[j*Nx+i]*u[j*Nx+i]+v[j*Nx+i]*v[j*Nx+i]);
        vmin=std::min(vv,vmin);
        vmax=std::max(vv,vmax);
        
        colors->InsertTuple1(k,vv);
        vectors->InsertTuple3(k, u[j*Nx+i], v[j*Nx+i], 0.0);
        k++;
      }
    vectors->Modified();
    colors->Modified();
    lut->SetTableRange(vmin,vmax);
    lut->Modified();

  }
  
}

#endif

