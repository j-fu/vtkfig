#ifndef VTKFIG_SURFACE_CONTOUR_H
#define VTKFIG_SURFACE_CONTOUR_H
#include "vtkDataSetAttributes.h"
#include "vtkGeometryFilter.h"
#include "vtkRectilinearGridGeometryFilter.h"


#include "vtkfigUnstructuredGridData.h"
#include "vtkfigRectilinearGridData.h"
#include "vtkfigContourBase.h"
#include "vtkfigTools.h"





namespace vtkfig
{

  
  class SurfaceContour: public ContourBase
  {


  public:
    SurfaceContour(): ContourBase(){};
    
    static std::shared_ptr<SurfaceContour> New() { return std::make_shared<SurfaceContour>();}
    
    virtual std::string SubClassName() {return std::string("SurfaceContour");}

    template< class G> void SetData(std::shared_ptr<G> xgriddata, const std::string xname)
    {
      state.spacedim=xgriddata->spacedim;
      data=xgriddata->griddata;
      name=xname;
    }
   
    
  private:
    vtkSmartPointer<vtkDataSet> data;
    std::string(name);

    virtual void RTBuild(
      vtkSmartPointer<vtkRenderWindow> window,
      vtkSmartPointer<vtkRenderWindowInteractor> interactor,
      vtkSmartPointer<vtkRenderer> renderer)
    {
      
      if (data->IsA("vtkUnstructuredGrid"))
      {
        auto griddata=vtkUnstructuredGrid::SafeDownCast(data);
        
        griddata->GetPointData()->SetActiveAttribute(name.c_str(),vtkDataSetAttributes::SCALARS);
        if (state.spacedim==2)
          ContourBase::RTBuild2D<vtkUnstructuredGrid,vtkGeometryFilter>(window, interactor,renderer,griddata);
        else
          ContourBase::RTBuild3D<vtkUnstructuredGrid,vtkGeometryFilter>(window,interactor,renderer,griddata); 
      }
      else if (data->IsA("vtkRectilinearGrid"))
      {
        auto griddata=vtkRectilinearGrid::SafeDownCast(data);
        
        griddata->GetPointData()->SetActiveAttribute(name.c_str(),vtkDataSetAttributes::SCALARS);

        if (state.spacedim==2)
          ContourBase::RTBuild2D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>(window, interactor,renderer,griddata);
        else
          ContourBase::RTBuild3D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>(window, interactor,renderer,griddata);
      }
    }
    
    // void ServerRTSend(vtkSmartPointer<Communicator> communicator)
    // {
    //   ContourBase::ServerRTSend(communicator, gridfunc, gridvalues);
    // }

    // void ClientMTReceive(vtkSmartPointer<Communicator> communicator)
    // {
    //   ContourBase::ClientMTReceive(communicator, gridfunc, gridvalues);
    // }
   
  };
  

}

#endif
