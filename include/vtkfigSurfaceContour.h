#ifndef VTKFIG_SURFACE_CONTOUR_H
#define VTKFIG_SURFACE_CONTOUR_H


#include "vtkSliderWidget.h"
#include "vtkSliderRepresentation.h"
#include "vtkRenderer.h"
#include "vtkCommand.h"


#include "vtkDataSetAttributes.h"
#include "vtkGeometryFilter.h"
#include "vtkRectilinearGridGeometryFilter.h"


#include "vtkfigUnstructuredGridData.h"
#include "vtkfigRectilinearGridData.h"
#include "vtkfigTools.h"
#include "vtkfigFigure.h"





namespace vtkfig
{

  
  class SurfaceContour: public Figure
  {


  public:
    SurfaceContour(): Figure()
    {
      sliderWidget = vtkSmartPointer<vtkSliderWidget>::New();
    }

    
    static std::shared_ptr<SurfaceContour> New() { return std::make_shared<SurfaceContour>();}
    
    virtual std::string SubClassName() {return std::string("SurfaceContour");}

    template< class G> void SetData(std::shared_ptr<G> xgriddata, const std::string xdataname)
    {
      state.spacedim=xgriddata->spacedim;
      data=xgriddata->griddata;
      dataname=xdataname;
      if (data->IsA("vtkUnstructuredGrid"))
        state.datatype=Figure::DataType::UnstructuredGrid;
      else  if (data->IsA("vtkRectilinearGrid"))
        state.datatype=Figure::DataType::RectilinearGrid;
    }
   


    
  private:

    vtkSmartPointer<vtkDataSet> data=NULL;
    std::string(dataname);

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
    
    
    vtkSmartPointer<vtkSliderWidget> sliderWidget;
    
    void AddSlider(vtkSmartPointer<vtkRenderWindowInteractor> i,vtkSmartPointer<vtkRenderer> r);
    
    friend class mySliderCallback;
   
  };
  

}

#endif
