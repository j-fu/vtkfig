#ifndef VTKFIG_TRICONTOUR2D_H
#define VTKFIG_TRICONTOUR2D_H

#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"
#include "vtkContourFilter.h"

#include "vtkfigFigure.h"
#include "vtkfigTools.h"



#include <vtkUnstructuredGrid.h>


namespace vtkfig
{

  
  class TriContour2D: public vtkfig::Figure
  {
  public:
    
    TriContour2D();
    static std::shared_ptr<TriContour2D> New() { return std::make_shared<TriContour2D>(); }
    
    
    void ShowSurface(bool b) {show_surface=b;}
    void ShowContour(bool b) {show_contour=b;}
    void ShowSurfaceColorbar(bool b) {show_surface_colorbar=b;}
    void ShowContourColorbar(bool b) {show_contour_colorbar=b;}
    
    void SetSurfaceRGBTable(RGBTable & tab, int tabsize)
    {
      surface_lut=BuildLookupTable(tab,tabsize);
    }
    void SetContourRGBTable(RGBTable & tab, int tabsize)
    {
      contour_lut=BuildLookupTable(tab,tabsize);
    }
    
    template <class V, class IV>
    void SetGrid(const V& points, 
                 const IV& cells);
    
    template <class V>
    void UpdateValues(const V&values);
    
    
    
    
    
  private:
    void RTBuild();
    double vmin,vmax;
    
    vtkSmartPointer<vtkUnstructuredGrid> gridfunc;
    vtkSmartPointer<vtkFloatArray> gridvalues;

    vtkSmartPointer<vtkLookupTable> surface_lut;
    vtkSmartPointer<vtkLookupTable> contour_lut;
    vtkSmartPointer<vtkContourFilter> isocontours;
    
    bool show_surface=true;
    bool show_contour=true;
    bool show_surface_colorbar=true;
    bool show_contour_colorbar=false;
    int ncont=10;
  };
  





  template <class V, class IV>
  inline
  void TriContour2D::SetGrid(const V& points, 
                            const IV& cells)
  {
    int npoints=points.size()/2;
    int ncells=cells.size()/3;
    
    
    gridfunc=vtkSmartPointer<vtkUnstructuredGrid>::New();
    
    for (int icell=0;icell<cells.size(); icell+=3)
    {
      vtkIdType 	 c[3]={cells[icell+0],cells[icell+1],cells[icell+2]};
      gridfunc->InsertNextCell(VTK_TRIANGLE,3,c);
    }
    
    vtkSmartPointer<vtkPoints> gridpoints = vtkSmartPointer<vtkPoints>::New();
    for (int ipoint=0;ipoint<points.size(); ipoint+=2)
    {
      gridpoints->InsertNextPoint(points[ipoint+0],points[ipoint+1],0);
    }
    gridfunc->SetPoints(gridpoints);
    
    
    gridvalues = vtkSmartPointer<vtkFloatArray>::New();
    gridvalues->SetNumberOfComponents(1);
    gridvalues->SetNumberOfTuples(npoints);
    for (int i=0;i<npoints; i++)
    {
      gridvalues->InsertComponent(i,0,0);
    }
    
    gridfunc->GetPointData()->SetScalars(gridvalues);
  }
  
  template <class V>
  inline
  void TriContour2D::UpdateValues(const V& values)
  {
    int npoints=values.size();
    for (int i=0;i<npoints; i++)
    {
        double v=values[i];
        vmin=std::min(v,vmin);
        vmax=std::max(v,vmax);
        gridvalues->InsertComponent(i,0,v);
    }
    gridvalues->Modified();
    surface_lut->SetTableRange(vmin,vmax);
    surface_lut->Modified();
    contour_lut->SetTableRange(vmin,vmax);
    contour_lut->Modified();

    double tempdiff = (vmax-vmin)/(10*ncont);
    isocontours->GenerateValues(ncont, vmin+tempdiff, vmax-tempdiff);
    isocontours->Modified();
  }


}

#endif
