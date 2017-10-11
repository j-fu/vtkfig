#ifndef VTKFIG_SCALARVIEW_H
#define VTKFIG_SCALARVIEW_H

#include "vtkRenderer.h"
#include "vtkCommand.h"

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
  class ScalarView: public Figure
  {
    
  public:
    ScalarView();
    static std::shared_ptr<ScalarView> New() { return std::make_shared<ScalarView>();}
    virtual std::string SubClassName()  override final {return std::string("ScalarView");}
    
    /// Toggle surface plot on plane
    void ShowSurface(bool b) {state.show_surface=b;}
    
    /// Toggle isoline rendering
    void ShowIsolines(bool b) {state.show_isolines=b;}
    
    /// Toggle isosurface rendering
    void ShowIsosurfaces(bool b) {state.show_isosurfaces=b;}
    
    /// Toggle colorbar visualization
    void ShowSurfaceColorbar(bool b) {state.show_surface_colorbar=b;}

    /// Set label format for colorbar
    void SetSurfaceColorbarLabelFormat(std::string fmt) {strncpy(state.surface_colorbar_label_format,fmt.c_str(),20);}

    /// Set number of labels in colorbar
    void SetSurfaceColorbarNumberOfLabels(int n) {state.surface_colorbar_num_labels=n;}

    
    /// Toggle elevation view  (yet to be implemented)
    void ShowElevation(bool b) {state.show_elevation=b;}
    
    /// Set number of isocontours to show
    void SetNumberOfIsocontours(int n) {isoline_filter->SetNumberOfContours(n);}
    
    /// Set width of isolines
    void SetIsolineWidth(double w) {state.isoline_width=w;}
    
    
    
    
    ///
    /// Set Surface RGB table from vector
    /// 
    /// \tparam V Vector class counting from zero with member functions
    ///  size() and operator[]. std::vector will work.
    /// 
    /// \param tab Vector containg data. 
    ///    One RGB point is described by x, r, g, b between 0 and 1
    ///    RGB point i is contained in '(tab[4*i],tab[4*i+1],tab[4*i+2],tab[4*i+3])
    /// \param lutsize  Size of lookup table created.
    template <class V> 
    void  SetSurfaceRGBTable(const V & tab, int lutsize);
    
    ///
    /// Set surface RGB table from RGBtable
    /// 
    /// \param tab RGB table containg data. 
    /// \param lutsize  Size of lookup table created.
    void SetSurfaceRGBTable(RGBTable & tab, int lutsize);
    

    
  private:
    
    void RTBuildVTKPipeline() override final;
    
    
    void ServerRTSend(vtkSmartPointer<internals::Communicator> communicator) override final;
    void ClientMTReceive(vtkSmartPointer<internals::Communicator> communicator) override final;
    
    
    template <class GRIDFUNC, class FILTER>
    void RTBuildVTKPipeline2D(vtkSmartPointer<GRIDFUNC> gridfunc);
    
    template <class GRIDFUNC, class FILTER>
    void RTBuildVTKPipeline3D(vtkSmartPointer<GRIDFUNC> gridfunc);
    
  };
  

////////////////////////////////////////////////////////////////

  template <class V> 
    inline
    void  ScalarView::SetSurfaceRGBTable(const V & tab, int lutsize)
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
    SetSurfaceRGBTable(rgbtab, lutsize);
  }
  



}




#endif
