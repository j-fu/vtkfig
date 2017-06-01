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
  
  /// 
  /// Surface and contour plot of 2/3D scalar data
  ///
  class SurfaceContour: public Figure
  {
    
  public:
    SurfaceContour();
    static std::shared_ptr<SurfaceContour> New() { return std::make_shared<SurfaceContour>();}
    virtual std::string SubClassName() {return std::string("SurfaceContour");}
    
    
  private:
    
    virtual void RTBuildVTKPipeline(
      vtkSmartPointer<vtkRenderWindow> window,
      vtkSmartPointer<vtkRenderWindowInteractor> interactor,
      vtkSmartPointer<vtkRenderer> renderer);
    
    
    void ServerRTSend(vtkSmartPointer<Communicator> communicator);
    
    void ClientMTReceive(vtkSmartPointer<Communicator> communicator);
    
    
    template <class GRIDFUNC, class FILTER>
      void RTBuildVTKPipeline2D(vtkSmartPointer<vtkRenderWindow> window,
                     vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                     vtkSmartPointer<vtkRenderer> renderer,
                     vtkSmartPointer<GRIDFUNC> gridfunc);
    
    template <class GRIDFUNC, class FILTER>
      void RTBuildVTKPipeline3D(vtkSmartPointer<vtkRenderWindow> window,
                     vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                     vtkSmartPointer<vtkRenderer> renderer,
                     vtkSmartPointer<GRIDFUNC> gridfunc);
    
  };
  
}

#endif
