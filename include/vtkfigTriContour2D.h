#ifndef VTKFIG_TRICONTOUR2D_H
#define VTKFIG_TRICONTOUR2D_H

#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"


#include "vtkfigFigure.h"
#include "vtkfigTools.h"



#include <vtkUnstructuredGrid.h>


namespace vtkfig
{

  
  class TriContour2D: public vtkfig::Figure
  {
  public:
    
    TriContour2D();
    
    // template<typename V>
    //     void Add(const V &xcoord, 
    //              const V &ycoord, 
    //              const V &values);
    
    
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
      void Add(const V& points, 
               const IV& cells, 
               const V& values)
    {
      int npoints=points.size()/2;
      int ncells=cells.size()/3;
      assert(npoints==values.size());
      
      vtkSmartPointer<vtkUnstructuredGrid> grid=vtkSmartPointer<vtkUnstructuredGrid>::New();
      
      for (int icell=0;icell<cells.size(); icell+=3)
      {
        vtkIdType 	 c[3]={cells[icell+0],cells[icell+1],cells[icell+2]};
        grid->InsertNextCell(VTK_TRIANGLE,3,c);
      }
      
      vtkSmartPointer<vtkPoints> gridpoints = vtkSmartPointer<vtkPoints>::New();
      for (int ipoint=0;ipoint<points.size(); ipoint+=2)
      {
        gridpoints->InsertNextPoint(points[ipoint+0],points[ipoint+1],0);
      }
      grid->SetPoints(gridpoints);
      
      
      vtkSmartPointer<vtkFloatArray> gridvalues = vtkSmartPointer<vtkFloatArray>::New();
      gridvalues->SetNumberOfComponents(1);
      gridvalues->SetNumberOfTuples(npoints);
      for (int i=0;i<npoints; i++)
      {
        gridvalues->InsertComponent(i,0,values[i]);
      }
      
      grid->GetPointData()->SetScalars(gridvalues);
      Add(grid);
    }
    
        
    private:
  void Add(vtkSmartPointer<vtkUnstructuredGrid> gridfunc);

      vtkSmartPointer<vtkLookupTable> surface_lut;
      vtkSmartPointer<vtkLookupTable> contour_lut;

      bool show_surface=true;
      bool show_contour=true;
      bool show_surface_colorbar=true;
      bool show_contour_colorbar=false;
  };
}


#endif
