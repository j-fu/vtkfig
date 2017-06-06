#ifndef VTKFIG_DOMAIN_H
#define VTKFIG_DOMAIN_H

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
  class Domain: public Figure
  {
    
  public:
    Domain();
    static std::shared_ptr<Domain> New() { return std::make_shared<Domain>();}
    virtual std::string SubClassName() {return std::string("Domain");}
    
    
  private:
    
    virtual void RTBuildVTKPipeline(
      vtkSmartPointer<vtkRenderWindow> window,
      vtkSmartPointer<vtkRenderWindowInteractor> interactor,
      vtkSmartPointer<vtkRenderer> renderer);
    
    
    void ServerRTSend(vtkSmartPointer<internals::Communicator> communicator);
    
    void ClientMTReceive(vtkSmartPointer<internals::Communicator> communicator);
    
    
    
    template <class GRIDFUNC, class FILTER>
      void RTBuildVTKPipeline(vtkSmartPointer<vtkRenderWindow> window,
                     vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                     vtkSmartPointer<vtkRenderer> renderer,
                     vtkSmartPointer<GRIDFUNC> gridfunc);
    
  };
  
}

#endif
