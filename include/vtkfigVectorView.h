#ifndef VTKFIG_QUIVER_H
#define VTKFIG_QUIVER_H

#include "vtkPolyData.h"
#include "vtkFloatArray.h"
#include "vtkfigFigure.h"
#include "vtkfigTools.h"

namespace vtkfig
{


  ///
  /// Experimental quiver view of vector fields
  ///
  class VectorView: public Figure
    {
    public:
      
      VectorView();
      static std::shared_ptr<VectorView> New() { return std::make_shared<VectorView>(); }
      virtual std::string SubClassName() {return std::string("VectorView");}
    

      /// Set scaling of arrows
      void SetQuiverArrowScale(double scale) { state.quiver_arrowscale_user=scale;}

      /// Set 
      void SetQuiverSurfaceDistance(double dist) { state.quiver_surface_distance=dist;}
      
      void SetQuiverGrid(int nx, int ny);

      void SetQuiverGrid(int nx, int ny, int nz);
      
      template <class V>
        void SetQuiverPoints( const V&p);
      

    private:
     
      void RTBuildVTKPipeline();
      void ServerRTSend(vtkSmartPointer<internals::Communicator> communicator);
      void ClientMTReceive(vtkSmartPointer<internals::Communicator> communicator);
      

      template <class DATA>
        void  RTBuildVTKPipeline(vtkSmartPointer<DATA> gridfunc);

        
      vtkSmartPointer<vtkPolyData> probePolyData;
      vtkSmartPointer<vtkLookupTable> lut;
      bool show_colorbar=false;

    };  
  

  template <class V>
    inline
    void VectorView::SetQuiverPoints( const V&p)
  {
    assert(data);
    auto probePoints =  vtkSmartPointer<vtkPoints>::New();
    if (state.spacedim==2)
      for (int i=0;i<p.size();i+=2)
        probePoints->InsertNextPoint (p[i+0],p[i+1],state.quiver_surface_distance);
    
    if (state.spacedim==3)
      for (int i=0;i<p.size();i+=3)
        probePoints->InsertNextPoint (p[i+0],p[i+1],p[i+2]);
    probePolyData =vtkSmartPointer<vtkPolyData>::New();
    probePolyData->SetPoints(probePoints);
  };
  
}




#endif

