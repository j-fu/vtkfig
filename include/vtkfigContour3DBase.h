#ifndef VTKFIG_CONTOUR3DBASE_H
#define VTKFIG_CONTOUR3DBASE_H
#include "vtkRenderer.h"
#include "vtkCommand.h"
#include "vtkfigFigure.h"
#include "vtkfigTools.h"
namespace vtkfig
{

    
 
  
  class Contour3DBase: public Figure
  {
  public:
    Contour3DBase()
    {
    }
    



  protected:
    

    template <class DATA>
      void ProcessData(    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                           vtkSmartPointer<vtkRenderer> renderer,
                           vtkSmartPointer<DATA> data);

  };






}
#endif
