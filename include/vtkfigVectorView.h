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


      ///
      /// Set quiver RGB table from vector
      /// 
      /// \tparam V Vector class counting from zero with member functions
      ///  size() and operator[]. std::vector will work.
      /// 
      /// \param tab Vector containg data. 
      ///    One RGB point is described by x, r, g, b between 0 and 1
      ///    RGB point i is contained in '(tab[4*i],tab[4*i+1],tab[4*i+2],tab[4*i+3])
      /// \param lutsize  Size of lookup table created.
      template <class V> 
        void  SetQuiverRGBTable(const V & tab, int lutsize);
      
      ///
      /// Set quiver RGB table from RGBtable
      /// 
      /// \param tab RGB table containg data. 
      /// \param lutsize  Size of lookup table created.
      void SetQuiverRGBTable(RGBTable & tab, int lutsize);
      
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
      vtkSmartPointer<vtkLookupTable> quiver_lut;
      RGBTable quiver_rgbtab;

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
  


////////////////////////////////////////////////////////////////
  template <class V> 
    inline
    void  VectorView::SetQuiverRGBTable(const V & tab, int lutsize)
  {
    RGBTable rgbtab;
    rgbtab.resize(tab.size()/4);
    for (int i=0,j=0; i<tab.size(); i+=4,j++)
    {
      rgbtab[j].x=tab[i+0];
      rgbtab[j].r=tab[i+1];
      rgbtab[j].g=tab[i+2];
      rgbtab[j].b=tab[i+3];
    }
    SetQuiverRGBTable(rgbtab, lutsize);
  }
  
}
 

#endif

