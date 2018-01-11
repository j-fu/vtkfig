#ifndef VTKFIG_GRID_VIEW_H
#define VTKFIG_GRID_VIEW_H

#include "vtkCommand.h"
#include "vtkDataSetAttributes.h"
#include "vtkGeometryFilter.h"
#include "vtkRectilinearGridGeometryFilter.h"
#include "vtkExtractGeometry.h"

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


    void ShowGridColorbar(bool b) { state.show_grid_colorbar=b;}

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

    vtkSmartPointer<vtkLookupTable> cell_lut;
    vtkSmartPointer<vtkLookupTable> bface_lut;
    RGBTable cell_rgbtab{
      {0.00, 1.0, 0.5, 0.5},
      {0.25, 1.0, 1.0, 0.5},
      {0.50, 0.5, 1.0, 0.5},
      {0.75, 0.5, 1.0, 1.0},
      {1.00, 0.5, 0.5, 1.0}};
    int cell_rgbtab_size=65;

    RGBTable bface_rgbtab{
      {0.00, 0.8, 0.0, 0.0},
      {0.25, 0.8, 0.8, 0.0},
      {0.50, 0.0, 0.8, 0.0},
      {0.75, 0.0, 0.8, 0.8},
      {1.00, 0.0, 0.0, 0.8}};
    int bface_rgbtab_size=65;


    vtkSmartPointer<vtkActor> cellplot=NULL;
    vtkSmartPointer<vtkActor> edgeplot=NULL;
    vtkSmartPointer<vtkScalarBarActor> cbar=NULL;

    vtkSmartPointer<vtkActor> bcellplot=NULL;
    vtkSmartPointer<vtkActor> bedgeplot=NULL;
    vtkSmartPointer<vtkScalarBarActor> bcbar=NULL;
    
    template <class GRIDFUNC, class FILTER>
      void RTBuildVTKPipeline2D();
    
    template <class GRIDFUNC, class FILTER>
      void RTBuildVTKPipeline3D();
    
  };
  
}

#endif
