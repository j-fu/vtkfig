#ifndef VTKFIG_QUIVER2D_H
#define VTKFIG_QUIVER2D_H

#include "vtkFloatArray.h"
#include "vtkfigFigure.h"
#include "vtkfigTools.h"

namespace vtkfig
{


  ///////////////////////////////////////////
  class Quiver: public Figure
    {
    public:
      
      Quiver();
      static std::shared_ptr<Quiver> New() { return std::make_shared<Quiver>(); }

      void SetArrowScale(double scale) {arrow_scale=scale;}

      template< class G> void SetData(G& xgriddata, const std::string xdataname);
      
      template< class G> void SetData(std::shared_ptr<G> xgriddata, const std::string xdataname);

    private:

      vtkSmartPointer<vtkDataSet> data=NULL;
      std::string dataname;
     
      virtual void RTBuild(
        vtkSmartPointer<vtkRenderWindow> window,
        vtkSmartPointer<vtkRenderWindowInteractor> interactor,
        vtkSmartPointer<vtkRenderer> renderer);
  
      vtkSmartPointer<vtkLookupTable> lut;
      bool show_colorbar=false;
      double arrow_scale=0.333;
    };  
  
  template< class G> inline void Quiver::SetData(G& xgriddata, const std::string xdataname)
  {
    state.spacedim=xgriddata.spacedim;
    data=xgriddata.griddata;
    dataname=xdataname;
    if (data->IsA("vtkUnstructuredGrid"))
      state.datatype=Figure::DataType::UnstructuredGrid;
    else  if (data->IsA("vtkRectilinearGrid"))
      state.datatype=Figure::DataType::RectilinearGrid;
  }
  
  
  template< class G> inline void Quiver::SetData(std::shared_ptr<G> xgriddata, const std::string xdataname)
  {
    SetData(*xgriddata,xdataname);
  }

  
}

#endif

