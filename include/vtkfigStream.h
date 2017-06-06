#ifndef VTKFIG_STREAM_H
#define VTKFIG_STREAM_H

#include "vtkPolyData.h"
#include "vtkFloatArray.h"
#include "vtkfigFigure.h"
#include "vtkfigTools.h"

namespace vtkfig
{


  ///
  /// Experimental quiver view of vector fields
  ///
  class Stream: public Figure
    {
    public:
      
      Stream();
      static std::shared_ptr<Stream> New() { return std::make_shared<Stream>(); }

      
      void SetStreamGrid(int nx, int ny);

      void SetStreamGrid(int nx, int ny, int nz);



      
      template <class V>
        void SetStreamPoints( const V&p);

      void SetStreamLineColor(double r, double b, double g) 
      {
        state.streamcolor[0]=r;
        state.streamcolor[1]=g;
        state.streamcolor[2]=b;
      }

      void SetStreamLineLenght(double l){ state.streamlength=l;}
      
      void SetStreamLineWidth(double w){state.streamribbonwidth=0.01;}
      
      
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
    void Stream::SetStreamPoints( const V&p)
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
