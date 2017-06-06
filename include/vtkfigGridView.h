#ifndef VTKFIG_GRID_VIEW_H
#define VTKFIG_GRID_VIEW_H

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
  /// Grid visualization.
  /// 
  /// This uses only the grid data from the DataSet. Optionally,
  /// the "cellregions" are used.
  class GridView: public Figure
  {
  public:
    GridView();
    static std::shared_ptr<GridView> New() { return std::make_shared<GridView>();}
    
    
  private:
    friend class Client;
    virtual std::string SubClassName() {return std::string("GridView");}
    
    virtual void RTBuildVTKPipeline();
    
    RGBTable grid_rgbtab{
      {0.00,1,0,0},
      {0.25,1,1,0},
      {0.50,0,1,0},
      {0.75,0,1,1},
      {1.00,0,0,1}};
    int grid_rgbtab_size=65;

    void ServerRTSend(vtkSmartPointer<internals::Communicator> communicator);
    
    void ClientMTReceive(vtkSmartPointer<internals::Communicator> communicator);
    
    
    template <class GRIDFUNC, class FILTER>
      void RTBuildVTKPipeline2D( vtkSmartPointer<GRIDFUNC> gridfunc);
    
    template <class GRIDFUNC, class FILTER>
      void RTBuildVTKPipeline3D(vtkSmartPointer<GRIDFUNC> gridfunc);
    
  };
  
}

#endif
