#ifndef VTKFIG_QUIVER2D_H
#define VTKFIG_QUIVER2D_H

#include "vtkFloatArray.h"
#include "vtkfigFigure.h"
#include "vtkfigTools.h"

namespace vtkfig
{


  ///////////////////////////////////////////
  class Quiver: public Figure
    {
    public:
      
      Quiver();
      static std::shared_ptr<Quiver> New() { return std::make_shared<Quiver>(); }

      void SetArrowScale(double scale) {arrow_scale=scale;}


    private:
     
      virtual void RTBuild(
        vtkSmartPointer<vtkRenderWindow> window,
        vtkSmartPointer<vtkRenderWindowInteractor> interactor,
        vtkSmartPointer<vtkRenderer> renderer);
  
      vtkSmartPointer<vtkLookupTable> lut;
      bool show_colorbar=false;
      double arrow_scale=0.333;
    };  
  
  
}

#endif

