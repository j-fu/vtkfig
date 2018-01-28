#ifndef VTKFIG_GRID_VIEW_H
#define VTKFIG_GRID_VIEW_H

#include <vtkCommand.h>
#include <vtkDataSetAttributes.h>
#include <vtkGeometryFilter.h>
#include <vtkRectilinearGridGeometryFilter.h>
#include <vtkExtractGeometry.h>

#include "vtkfigDataSet.h"
#include "vtkfigTools.h"
#include "vtkfigFigure.h"



namespace vtkfig
{
  
  /// 
  /// Grid visualization.
  /// 
  /// This uses only the grid data from the DataSet. Optionally,
  /// the "cellregions" are used.
  class GridView: public Figure
  {
  public:
    GridView();
    static std::shared_ptr<GridView> New() { return std::make_shared<GridView>();}
    std::string SubClassName() override final {return std::string("GridView");}


    void ShowGridColorbar(bool b) { Figure::state.show_grid_colorbar=b;}
    void ShowGridEdges(bool b) { Figure::state.show_grid_edges=b;}

    /// Process keyboard and mouse move events
    void RTProcessKey(const std::string key) override final;
    void RTProcessMove(int dx, int dy) override final;

    void RTShowPlanePos(vtkSmartPointer<vtkPlane> planeXYZ, const std::string plane, int idim);
    
    int RTProcessPlaneKey(
      const std::string plane,
      int idim,
      const std::string key,  
      bool & edit, 
      vtkSmartPointer<vtkPlane> planeXYZ);

    int RTProcessPlaneMove(const std::string plane,int idim, int dx, int dy, bool & edit, 
                                   vtkSmartPointer<vtkPlane> planeXYZ );

    
  private:
    friend class Client;
    
    void RTBuildVTKPipeline() override final;
    vtkSmartPointer<vtkExtractGeometry> cutgeometry;
    vtkSmartPointer<vtkExtractGeometry> bcutgeometry=NULL;



    vtkSmartPointer<vtkActor> cellplot=NULL;
    vtkSmartPointer<vtkActor> edgeplot=NULL;

    vtkSmartPointer<vtkActor> bcellplot=NULL;
    vtkSmartPointer<vtkActor> bedgeplot=NULL;
    
    template <class GRIDFUNC, class FILTER>
      void RTBuildVTKPipeline2D();
    
    template <class GRIDFUNC, class FILTER>
      void RTBuildVTKPipeline3D();
    
  };
  
}

#endif
