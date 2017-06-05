#ifndef VTKFIG_BOUNDARY_H
#define VTKFIG_BOUNDARY_H

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
  class Boundary: public Figure
  {
    
  public:
    Boundary();
    static std::shared_ptr<Boundary> New() { return std::make_shared<Boundary>();}
    virtual std::string SubClassName() {return std::string("Boundary");}
    
    
  private:
    
    virtual void RTBuildVTKPipeline(
      vtkSmartPointer<vtkRenderWindow> window,
      vtkSmartPointer<vtkRenderWindowInteractor> interactor,
      vtkSmartPointer<vtkRenderer> renderer);
    
    
    void ServerRTSend(vtkSmartPointer<internals::Communicator> communicator);
    
    void ClientMTReceive(vtkSmartPointer<internals::Communicator> communicator);
    
    
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
