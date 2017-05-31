#ifndef VTKFIG_GRID_VIEW_H
#define VTKFIG_GRID_VIEW_H

#include "vtkSliderWidget.h"
#include "vtkSliderRepresentation.h"
#include "vtkRenderer.h"
#include "vtkCommand.h"

#include "vtkDataSetAttributes.h"
#include "vtkGeometryFilter.h"
#include "vtkRectilinearGridGeometryFilter.h"

#include "vtkfigDataSet.h"
#include "vtkfigTools.h"
#include "vtkfigFigure.h"



namespace vtkfig
{
  
  class GridView: public Figure
  {
    
  public:
    GridView();
    static std::shared_ptr<GridView> New() { return std::make_shared<GridView>();}
    virtual std::string SubClassName() {return std::string("GridView");}
    
    
  private:
    
    virtual void RTBuild(
      vtkSmartPointer<vtkRenderWindow> window,
      vtkSmartPointer<vtkRenderWindowInteractor> interactor,
      vtkSmartPointer<vtkRenderer> renderer);
    
    RGBTable grid_rgbtab{
      {0.00,1,0,0},
      {0.25,1,1,0},
      {0.50,0,1,0},
      {0.75,0,1,1},
      {1.00,0,0,1}};
    int grid_rgbtab_size=65;

    void ServerRTSend(vtkSmartPointer<Communicator> communicator);
    
    void ClientMTReceive(vtkSmartPointer<Communicator> communicator);
    
    
    template <class GRIDFUNC, class FILTER>
      void RTBuild2D(vtkSmartPointer<vtkRenderWindow> window,
                     vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                     vtkSmartPointer<vtkRenderer> renderer,
                     vtkSmartPointer<GRIDFUNC> gridfunc);
    
    template <class GRIDFUNC, class FILTER>
      void RTBuild3D(vtkSmartPointer<vtkRenderWindow> window,
                     vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                     vtkSmartPointer<vtkRenderer> renderer,
                     vtkSmartPointer<GRIDFUNC> gridfunc);
    
    vtkSmartPointer<vtkSliderWidget> sliderWidget;
       
  };
  
}

#endif
