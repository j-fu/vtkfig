#ifndef VTKFIG_QUIVER_H
#define VTKFIG_QUIVER_H

#include <vtkPolyData.h>
#include <vtkDoubleArray.h>

#include "vtkfigFigure.h"
#include "vtkfigTools.h"



namespace vtkfig
{


  ///
  /// Quiver plots + streamlines for vector fields
  ///
  class VectorView: public Figure
    {
    public:
      
      VectorView();
      static std::shared_ptr<VectorView> New() { return std::make_shared<VectorView>(); }
      virtual std::string SubClassName() override final {return std::string("VectorView");}
    

      /// Show quiver plot (true by default)
      void ShowQuiver(bool b) { state.show_quiver=b;}

      /// Set scaling of arrows
      void SetQuiverArrowScale(double scale) { state.quiver_arrowscale_user=scale;}

      /// Show color bar for quiver colors
      void ShowQuiverColorbar(bool b) { state.show_quiver_colorbar=b;}

      /// Set quiver points on grid in domain bounding box (2D)
      void SetQuiverGrid(int nx, int ny);

      /// Set quiver points on grid in domain bounding box (3D)
      void SetQuiverGrid(int nx, int ny, int nz);
      
      /// Set quiver points from vector
      template <class V>
        void SetQuiverPoints( const V&p);
      


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
      

      /// Show stream lines (false by default)
      void ShowStreamLines(bool b) { state.show_stream=b;}

      /// Show color bar for stream lines
      void ShowStreamLineColorbar(bool b) { state.show_stream_colorbar=b;}

      /// Set length of stream lines
      void SetStreamLineMaximumPropagation(double l){ state.stream_maximum_propagation=l;}

      /// Set maximum number of steps
      void SetStreamLineMaximumNumberOfSteps(int l){ state.stream_maximum_number_of_steps=l;}

      /// Set initial integration step for streamlines
      void SetStreamLineInitialIntegrationStep(double l){ state.stream_initial_integration_step=l;}
      
      /// Set  maximum integration step for streamlines
      void SetStreamLineMaximumIntegrationStep(double l){ state.stream_maximum_integration_step=l;}
      
      /// Set width of stream ribbons
      void SetStreamLineWidth(double w){state.stream_ribbonwidth=w;}

      
      /// Set seed points for streamlines
      template <class V>
        void SetStreamLineSeedPoints( const V&p);


      ///
      /// Set streamline RGB table from vector
      /// 
      /// \tparam V Vector class counting from zero with member functions
      ///  size() and operator[]. std::vector will work.
      /// 
      /// \param tab Vector containg data. 
      ///    One RGB point is described by x, r, g, b between 0 and 1
      ///    RGB point i is contained in '(tab[4*i],tab[4*i+1],tab[4*i+2],tab[4*i+3])
      /// \param lutsize  Size of lookup table created.
      template <class V> 
        void  SetStreamLineRGBTable(const V & tab, int lutsize);
      
      ///
      /// Set quiver RGB table from RGBtable
      /// 
      /// \param tab RGB table containg data. 
      /// \param lutsize  Size of lookup table created.
      void SetStreamLineRGBTable(RGBTable & tab, int lutsize);
      



    private:
     
      void RTBuildVTKPipeline() override final;
      void ServerRTSend(vtkSmartPointer<internals::Communicator> communicator) override final;
      void ClientMTReceive(vtkSmartPointer<internals::Communicator> communicator) override final;
      

      template <class DATA> void  RTBuildVTKPipeline0();

        
      vtkSmartPointer<vtkPolyData> probePolyData;
      vtkSmartPointer<vtkPolyData> seedPolyData;
    };  
  

  template <class V>
    inline
    void VectorView::SetQuiverPoints( const V&p)
  {

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

  template <class V> 
    inline
    void  VectorView::SetStreamLineRGBTable(const V & tab, int lutsize)
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
    SetStreamLineRGBTable(rgbtab, lutsize);
  }


  template <class V>
    inline
    void VectorView::SetStreamLineSeedPoints( const V&p)
  {

    auto seedPoints =  vtkSmartPointer<vtkPoints>::New();
    if (state.spacedim==2)
      for (int i=0;i<p.size();i+=2)
        seedPoints->InsertNextPoint (p[i+0]*coordinate_scale_factor,p[i+1]*coordinate_scale_factor,0);
    
    if (state.spacedim==3)
      for (int i=0;i<p.size();i+=3)
        seedPoints->InsertNextPoint (p[i+0]*coordinate_scale_factor,
                                     p[i+1]*coordinate_scale_factor,
                                     p[i+2]*coordinate_scale_factor);
    seedPolyData =vtkSmartPointer<vtkPolyData>::New();
    seedPolyData->SetPoints(seedPoints);
  };
  
  
}
 

#endif

