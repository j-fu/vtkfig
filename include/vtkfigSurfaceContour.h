#ifndef VTKFIG_SURFACE_CONTOUR_H
#define VTKFIG_SURFACE_CONTOUR_H

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
  class SurfaceContour: public Figure
  {
    
  public:
    SurfaceContour();
    static std::shared_ptr<SurfaceContour> New() { return std::make_shared<SurfaceContour>();}
    virtual std::string SubClassName() {return std::string("SurfaceContour");}
    
    /// Set Range of values
    void SetValueRange(double vmin, double vmax){state.vmin_set=vmin; state.vmax_set=vmax;}
    
    /// Toggle surface plot on plane
    void ShowSurface(bool b) {state.show_surface=b;}
    
    /// Toggle isoline rendering
    void ShowIsolines(bool b) {state.show_isolines=b;}
    
    /// Toggle isosurface rendering
    void ShowIsosurfaces(bool b) {state.show_isosurfaces=b;}
    
    /// Toggle colorbar visualization
    void ShowSurfaceColorbar(bool b) {state.show_surface_colorbar=b;}
    
    /// Toggle elevation view  (yet to be implemented)
    void ShowElevation(bool b) {state.show_elevation=b;}
    
    /// Set number of isocontours to show
    void SetNumberOfIsocontours(int n) {state.num_contours=n; state.max_num_contours= std::max(n,state.max_num_contours);}
    
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
    
    virtual void RTBuildVTKPipeline();
    
    
    void ServerRTSend(vtkSmartPointer<internals::Communicator> communicator);
    
    void ClientMTReceive(vtkSmartPointer<internals::Communicator> communicator);
    
    
    template <class GRIDFUNC, class FILTER>
    void RTBuildVTKPipeline2D(vtkSmartPointer<GRIDFUNC> gridfunc);
    
    template <class GRIDFUNC, class FILTER>
    void RTBuildVTKPipeline3D(vtkSmartPointer<GRIDFUNC> gridfunc);
    
  };
  

////////////////////////////////////////////////////////////////
  template <class V> 
    inline
    void  SurfaceContour::SetSurfaceRGBTable(const V & tab, int lutsize)
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
