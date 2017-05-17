#ifndef VTKFIG_CONTOUR2DBASE_H
#define VTKFIG_CONTOUR2DBASE_H
#include "vtkSliderWidget.h"
#include "vtkRenderer.h"
#include "vtkSliderRepresentation.h"
#include "vtkCommand.h"
#include "vtkfigFigure.h"
#include "vtkfigTools.h"
namespace vtkfig
{

    
 
  
  class Contour2DBase: public Figure
  {
  public:
    Contour2DBase()
    {
      sliderWidget = vtkSmartPointer<vtkSliderWidget>::New();
    }
  



  protected:
    vtkSmartPointer<vtkSliderWidget> sliderWidget;
    

    template <class DATA, class FILTER>
    void ProcessData(    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                         vtkSmartPointer<vtkRenderer> renderer,
                         vtkSmartPointer<DATA> data);

    void AddSlider(vtkSmartPointer<vtkRenderWindowInteractor> i,vtkSmartPointer<vtkRenderer> r);

    friend class mySliderCallback;

    
  };






}
#endif
