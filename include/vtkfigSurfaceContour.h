#ifndef VTKFIG_SURFACE_CONTOUR_H
#define VTKFIG_SURFACE_CONTOUR_H

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
  
  class SurfaceContour: public Figure
  {
    
  public:
    SurfaceContour();
    static std::shared_ptr<SurfaceContour> New() { return std::make_shared<SurfaceContour>();}
    virtual std::string SubClassName() {return std::string("SurfaceContour");}
    
    
  private:
    
    virtual void RTBuild(
      vtkSmartPointer<vtkRenderWindow> window,
      vtkSmartPointer<vtkRenderWindowInteractor> interactor,
      vtkSmartPointer<vtkRenderer> renderer);
    
    
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
    
  };
  
}

#endif
