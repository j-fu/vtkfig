/**
    \file vtkfigGridView.h

    Provide user API  class vtkfig::GridView derived from vtkfig::Figure for visualizing 2D/3D discretization grids.
    
*/

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
  
  /**
     Grid visualization.
     
     This uses only the grid data from the DataSet. Optionally,
     the "cellregions" are used.
     
  */
  class GridView: public Figure
  {
    


    GridView();

  public:
    

    /// GridView destructor.
    ~GridView(){};

    /// GridView smart pointer constructor.
    static std::shared_ptr<GridView> New()
    {
      struct make_shared_enabler : public GridView {};
      return std::make_shared<make_shared_enabler>();
    }



    std::string SubClassName() override final {return std::string("GridView");}

    /// Toggle visibility of grid colorbar
    void ShowGridColorbar(bool b) { Figure::state.show_grid_colorbar=b;}

    /// Toggle visibility of grid edges
    void ShowGridEdges(bool b) { Figure::state.show_grid_edges=b;}

  private:
    friend class Client;
    friend class MainThread;
    
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

