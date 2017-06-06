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
  class Quiver: public Figure
    {
    public:
      
      Quiver();
      static std::shared_ptr<Quiver> New() { return std::make_shared<Quiver>(); }

      void SetQuiverArrowScale(double scale) { state.qv_arrow_scale=scale;}
      
      void SetQuiverGrid(int nx, int ny);

      void SetQuiverGrid(int nx, int ny, int nz);
      
      template <class V>
        void SetQuiverPoints( const V&p);
      

    private:
     
      virtual void RTBuildVTKPipeline(
        vtkSmartPointer<vtkRenderWindow> window,
        vtkSmartPointer<vtkRenderWindowInteractor> interactor,
        vtkSmartPointer<vtkRenderer> renderer);
      

      template <class DATA>
        void  RTBuildVTKPipeline(
          vtkSmartPointer<vtkRenderWindow> window,
          vtkSmartPointer<vtkRenderWindowInteractor> interactor,
          vtkSmartPointer<vtkRenderer> renderer,
          vtkSmartPointer<DATA> gridfunc);

        
      vtkSmartPointer<vtkPolyData> probePolyData;
      vtkSmartPointer<vtkLookupTable> lut;
      bool show_colorbar=false;
    };  
  

  template <class V>
    inline
    void Quiver::SetQuiverPoints( const V&p)
  {
    assert(data);
    auto probePoints =  vtkSmartPointer<vtkPoints>::New();
    if (state.spacedim==2)
      for (int i=0;i<p.size();i+=2)
        probePoints->InsertNextPoint (p[i+0],p[i+1],0);
    
    if (state.spacedim==3)
      for (int i=0;i<p.size();i+=3)
        probePoints->InsertNextPoint (p[i+0],p[i+1],p[i+2]);
    probePolyData =vtkSmartPointer<vtkPolyData>::New();
    probePolyData->SetPoints(probePoints);
  };
  
}




#endif

