#ifndef VTKFIG_STREAM_H
#define VTKFIG_STREAM_H

#include "vtkPolyData.h"
#include "vtkFloatArray.h"
#include "vtkfigFigure.h"
#include "vtkfigTools.h"

namespace vtkfig
{


  ///
  /// Experimental streamline view of vector fields
  ///
  class Stream: public Figure
    {
    public:
      
      Stream();
      static std::shared_ptr<Stream> New() { return std::make_shared<Stream>(); }
      virtual std::string SubClassName() {return std::string("Stream");}
      
      /// Set color of streamlines
      void SetStreamLineColor(double r, double b, double g) 
      {
        state.streamcolor[0]=r;
        state.streamcolor[1]=g;
        state.streamcolor[2]=b;
      }
      
      /// Set length of stream lines
      void SetStreamLineLength(double l){ state.streamlength=l;}
      
      /// Set width of stream ribbons
      void SetStreamLineWidth(double w){state.streamribbonwidth=0.01;}

      
      /// Set seed points
      template <class V>
        void SetSeedPoints( const V&p);

      
    private:

      virtual void RTBuildVTKPipeline();
      void ServerRTSend(vtkSmartPointer<internals::Communicator> communicator);
      void ClientMTReceive(vtkSmartPointer<internals::Communicator> communicator);
      

      template <class DATA>
        void  RTBuildVTKPipeline(vtkSmartPointer<DATA> gridfunc);

        
      vtkSmartPointer<vtkPolyData> seedPolyData;
      vtkSmartPointer<vtkLookupTable> lut;
      bool show_colorbar=false;
    };  
  

  template <class V>
    inline
    void Stream::SetSeedPoints( const V&p)
  {
    assert(data);
    auto seedPoints =  vtkSmartPointer<vtkPoints>::New();
    if (state.spacedim==2)
      for (int i=0;i<p.size();i+=2)
        seedPoints->InsertNextPoint (p[i+0],p[i+1],0);
    
    if (state.spacedim==3)
      for (int i=0;i<p.size();i+=3)
        seedPoints->InsertNextPoint (p[i+0],p[i+1],p[i+2]);
    seedPolyData =vtkSmartPointer<vtkPolyData>::New();
    seedPolyData->SetPoints(seedPoints);
  };
  
}




#endif

