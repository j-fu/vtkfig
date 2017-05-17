#ifndef VTKFIG_CONTOURBASE_H
#define VTKFIG_CONTOURBASE_H
#include "vtkSliderWidget.h"
#include "vtkRenderer.h"
#include "vtkSliderRepresentation.h"
#include "vtkCommand.h"
#include "vtkFloatArray.h"

#include "vtkfigFigure.h"
#include "vtkfigTools.h"
namespace vtkfig
{
  
  class ContourBase: public Figure
  {
  protected:
  ContourBase(): Figure()
    {
      sliderWidget = vtkSmartPointer<vtkSliderWidget>::New();
    }
  

    template <class GRIDFUNC, class FILTER>
      void RTBuild2D(vtkSmartPointer<vtkRenderWindow> window,
                     vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                     vtkSmartPointer<vtkRenderer> renderer,
                     vtkSmartPointer<GRIDFUNC> gridfunc);
    
    template <class GRIDFUNC>
      void RTBuild3D(vtkSmartPointer<vtkRenderWindow> window,
                     vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                     vtkSmartPointer<vtkRenderer> renderer,
                     vtkSmartPointer<GRIDFUNC> gridfunc);
    
    template <class GRIDFUNC>
      void ServerRTSend(vtkSmartPointer<Communicator> communicator,vtkSmartPointer<GRIDFUNC> gridfunc, vtkSmartPointer<vtkFloatArray> values);
    
    template <class GRIDFUNC>
      void ClientMTReceive(vtkSmartPointer<Communicator> communicator,vtkSmartPointer<GRIDFUNC> gridfunc, vtkSmartPointer<vtkFloatArray> values);
    
    
    vtkSmartPointer<vtkSliderWidget> sliderWidget;
    
    void AddSlider(vtkSmartPointer<vtkRenderWindowInteractor> i,vtkSmartPointer<vtkRenderer> r);
    
    friend class mySliderCallback;
    
    int grid_changed=0;
    bool has_data=false;


  };






}

#endif
