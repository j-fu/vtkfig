#ifndef VTKFIG_QUIVER2D_H
#define VTKFIG_QUIVER2D_H

#include "vtkFloatArray.h"
#include "vtkfigFigure.h"
#include "vtkfigTools.h"

namespace vtkfig
{


  ///
  /// Experimental quiver view of vector fields
  ///
  class Quiver: public Figure
    {
    public:
      
      Quiver();
      static std::shared_ptr<Quiver> New() { return std::make_shared<Quiver>(); }

    private:
     
      virtual void RTBuildVTKPipeline(
        vtkSmartPointer<vtkRenderWindow> window,
        vtkSmartPointer<vtkRenderWindowInteractor> interactor,
        vtkSmartPointer<vtkRenderer> renderer);
      
      template <class DATA, class FILTER>
        void  RTBuildVTKPipeline2D(
          vtkSmartPointer<vtkRenderWindow> window,
          vtkSmartPointer<vtkRenderWindowInteractor> interactor,
          vtkSmartPointer<vtkRenderer> renderer,
          vtkSmartPointer<DATA> gridfunc);
        
  
      vtkSmartPointer<vtkLookupTable> lut;
      bool show_colorbar=false;
    };  
  
  
}

#endif

