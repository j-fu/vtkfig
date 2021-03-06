#include <vtkSliderRepresentation2D.h>
#include <vtkProperty2D.h>
#include <vtkAlgorithmOutput.h>
#include <vtkTextProperty.h>
#include <vtkRectilinearGrid.h>
#include <vtkGeometryFilter.h>
#include <vtkRectilinearGridGeometryFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridGeometryFilter.h>
#include <vtkPlane.h>
#include <vtkCutter.h>
#include <vtkImplicitBoolean.h>
#include <vtkOutlineFilter.h>
#include <vtkCubeAxesActor2D.h>
#include <vtkAppendPolyData.h>
#include <vtkAssignAttribute.h>
#include <vtkCamera.h>
#include <vtkTextActor.h>
#include <vtkCornerAnnotation.h>
#include <vtkCoordinate.h>
#include <vtkExtractEdges.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransformFilter.h>



#include "vtkfigGridView.h"
#include "config.h"


namespace vtkfig
{


  /////////////////////////////////////////////////////////////////////
  /// Constructor
  GridView::GridView(): Figure()
  {
    cutgeometry=vtkSmartPointer<vtkExtractGeometry>::New();
    cutgeometry->SetImplicitFunction(planeZ);
    bcutgeometry=vtkSmartPointer<vtkExtractGeometry>::New();
    bcutgeometry->SetImplicitFunction(planeZ);
  }

  std::shared_ptr<GridView> GridView::New()
  {
    struct make_shared_enabler : public GridView {};
    return std::make_shared<make_shared_enabler>();
  }

  
  int GridView::RTProcessPlaneMove(const std::string plane,int idim, int dx, int dy, bool & edit, 
                                  vtkSmartPointer<vtkPlane> planeXYZ )
  {
    if (edit)
    {
      double trans_origin[3];
      planeXYZ->GetOrigin(trans_origin);
      double planepos=trans_origin[idim];
      double delta=trans_bounds[2*idim+1]-trans_bounds[2*idim+0];
      planepos+=(0.01+1.0e-8)*((double)dx)*delta;
      planepos=std::min(planepos,trans_bounds[2*idim+1]+1.0e-10*delta);
      planepos=std::max(planepos,trans_bounds[2*idim+0]-1.0e-10*delta);
      trans_origin[idim]=planepos;
      planeXYZ->SetOrigin(trans_origin);
      RTShowPlanePos(planeXYZ,plane,idim);
      planeXYZ->Modified();
      cutgeometry->Modified();
      bcutgeometry->Modified();
      return 1;
    }
    return 0;
  }
  
  
  void GridView::RTProcessMove(int dx, int dy)
  {
    if (RTProcessPlaneMove("x",0,dx,dy,edit.x_plane, planeX)) return;
    if (RTProcessPlaneMove("y",1,dx,dy,edit.y_plane, planeY)) return;
    if (RTProcessPlaneMove("z",2,dx,dy,edit.z_plane, planeZ)) return;
  }


  void GridView::RTShowPlanePos(vtkSmartPointer<vtkPlane> planeXYZ, const std::string plane, int idim)
  {
    double trans_origin[3];
    planeXYZ->GetOrigin(trans_origin);
    double planepos=data_bounds[2*idim]+ (data_bounds[2*idim+1]-data_bounds[2*idim])*(trans_origin[idim]-trans_bounds[2*idim])/(trans_bounds[2*idim+1]-trans_bounds[2*idim]);
    RTMessage("plane_" + plane+"=" + std::to_string(planepos));
  }


  int GridView::RTProcessPlaneKey(
    const std::string plane,
    int idim,
    const std::string key,  
    bool & edit, 
    vtkSmartPointer<vtkPlane> planeXYZ)
  {
    
    if (!edit && key==plane)
    {
      edit=true;     
      RTShowPlanePos(planeXYZ,plane,idim);
      cutgeometry->SetImplicitFunction(planeXYZ);
      cutgeometry->Modified();
      bcutgeometry->SetImplicitFunction(planeXYZ);
      bcutgeometry->Modified();
      return 1;
    }

    if (edit&& key=="Escape")
    {
      edit=false;
      return 1;
    }
    
    return 0;
  }
  
  
  void GridView::RTProcessKey(const std::string key)
  {
    
    if (key=="slash")
    {
      cutgeometry->SetExtractInside(!cutgeometry->GetExtractInside());
      cutgeometry->Modified();
      bcutgeometry->SetExtractInside(!bcutgeometry->GetExtractInside());
      bcutgeometry->Modified();
      return;
    }

    if (key=="A")
    {
      state.show_domain_axes=!state.show_domain_axes;
      state.show_domain_box=state.show_domain_axes;
      RTRefreshState();
      return;
    }
    
    if (key=="O")
    {
      state.show_domain_outline=!state.show_domain_outline;
      RTRefreshState();
      return;
    }



    if (key=="C" && cellplot)
    {
      int vis=cellplot->GetVisibility();
      vis=!vis;
      cellplot->SetVisibility(vis);
      if (edgeplot)
        edgeplot->SetVisibility(vis);

      if (cbar)
        cbar->SetVisibility(vis);
      return;
    }

    if (key=="B" && bcellplot)
    {
      int vis=bcellplot->GetVisibility();
      vis=!vis;
      bcellplot->SetVisibility(vis);
      if (bedgeplot)
        bedgeplot->SetVisibility(vis);
      if (bcbar)
        bcbar->SetVisibility(vis);
      return;
    }


    if (
      (edit.x_plane||edit.y_plane|| edit.z_plane)
      &&(key=="x"|| key=="y"|| key=="z")
      )
    {
      edit.x_plane=false;
      edit.y_plane=false;
      edit.z_plane=false;
    }

    if (RTProcessPlaneKey("x",0,key,edit.x_plane, planeX)) return;
    if (RTProcessPlaneKey("y",1,key,edit.y_plane, planeY)) return;
    if (RTProcessPlaneKey("z",2,key,edit.z_plane, planeZ)) return;
    

  }


  /////////////////////////////////////////////////////////////////////
  /// 2D Filter

  template <class DATA, class FILTER>
  void GridView::RTBuildVTKPipeline2D()
  {
    RTCalcTransform();
    {
      auto cr=vtkDoubleArray::SafeDownCast(DATA::SafeDownCast(data_producer->GetOutputDataObject(0))->GetCellData()->GetAbstractArray("cellregions"));
      
      auto scalar = vtkSmartPointer<vtkAssignAttribute>::New();
      if (cr)
      {
        scalar->Assign("cellregions",vtkDataSetAttributes::SCALARS,vtkAssignAttribute::CELL_DATA);
        scalar->SetInputConnection(data_producer->GetOutputPort());
      }
      
      auto geometry=vtkSmartPointer<FILTER>::New();
      if (cr)
        geometry->SetInputConnection(scalar->GetOutputPort());
      else
        geometry->SetInputConnection(data_producer->GetOutputPort());
      
      
      
      auto transgeometry=vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      transgeometry->SetInputConnection(geometry->GetOutputPort());
      transgeometry->SetTransform(transform);
      
      auto  cells = vtkSmartPointer<vtkPolyDataMapper>::New();
      cells->SetInputConnection(transgeometry->GetOutputPort());
      
      if (cr)
      {
        cells->UseLookupTableScalarRangeOn();
        cells->SetLookupTable(cell_lut);
      }
      else
        cells->ScalarVisibilityOff();

#ifdef VTK_HAS_MAPPER_IMMEDIATE_RENDERING_ON
      cells->ImmediateModeRenderingOn();
#endif
      cellplot = vtkSmartPointer<vtkActor>::New();
      cellplot->SetMapper(cells);
      if (!cr)
        cellplot->GetProperty()->SetColor(0.9,0.9,0.9);
      Figure::RTAddActor(cellplot);
      

      
      if (Figure::state.show_grid_edges)
      {
        // Extract edges is slow for large datasets so it is better to
        // plot cells twice: once with, once without wireframe.
        
        // auto edges= vtkSmartPointer<vtkExtractEdges>::New();
        // edges->SetInputConnection(transgeometry->GetOutputPort());
        // auto  emapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        // emapper->SetInputConnection(edges->GetOutputPort());
        // emapper->ScalarVisibilityOff();
        
        // edgeplot = vtkSmartPointer<vtkActor>::New();
        // edgeplot->GetProperty()->SetColor(0,0,0);
        // edgeplot->SetMapper(emapper);
        // Figure::RTAddActor(edgeplot);
        
        
        auto  celledges = vtkSmartPointer<vtkPolyDataMapper>::New();
        celledges->SetInputConnection(transgeometry->GetOutputPort());
        celledges->ScalarVisibilityOff();
#ifdef VTK_HAS_MAPPER_IMMEDIATE_RENDERING_ON
        celledges->ImmediateModeRenderingOn();
#endif
        auto celledgeplot = vtkSmartPointer<vtkActor>::New();
        celledgeplot->SetMapper(celledges);
        celledgeplot->GetProperty()->SetColor(0,0,0);
        celledgeplot->GetProperty()->SetRepresentationToWireframe();
        Figure::RTAddActor(celledgeplot);
      }

      if (cr && state.show_grid_colorbar)
      {
        cbar=internal::BuildColorBar(cells);
        cbar->SetTitle("C    ");
        cbar->SetLabelFormat(" %-2.0f     ");
        double range[2];
        cell_lut->GetTableRange(range);
        cbar->SetNumberOfLabels((int)(range[1]-range[0]+1));
        Figure::RTAddActor2D(cbar);
      }
    }

    
    auto boundary_data=vtkDataSet::SafeDownCast(boundary_data_producer->GetOutputDataObject(0));
    if (boundary_data)
    {

      auto bcr=vtkDoubleArray::SafeDownCast(boundary_data->GetCellData()->GetAbstractArray("boundarycellregions"));
      if (bcr)
      {
        auto bscalar = vtkSmartPointer<vtkAssignAttribute>::New();
        bscalar->Assign("boundarycellregions",vtkDataSetAttributes::SCALARS,vtkAssignAttribute::CELL_DATA);
        bscalar->SetInputConnection(boundary_data_producer->GetOutputPort());

        auto bgeometry=vtkSmartPointer<FILTER>::New();
        bgeometry->SetInputConnection(bscalar->GetOutputPort());
        
        auto transbgeometry=vtkSmartPointer<vtkTransformPolyDataFilter>::New();
        transbgeometry->SetInputConnection(bgeometry->GetOutputPort());
        transbgeometry->SetTransform(transform);
        
        auto  bcells = vtkSmartPointer<vtkPolyDataMapper>::New();
        bcells->SetInputConnection(transbgeometry->GetOutputPort());
        
        bcells->UseLookupTableScalarRangeOn();
        bcells->SetLookupTable(bface_lut);
        bcells->ScalarVisibilityOn();
#ifdef VTK_HAS_MAPPER_IMMEDIATE_RENDERING_ON
        bcells->ImmediateModeRenderingOn();
#endif
        bcellplot = vtkSmartPointer<vtkActor>::New();
        bcellplot->SetMapper(bcells);
        bcellplot->GetProperty()->SetLineWidth(5);
        Figure::RTAddActor(bcellplot);
        
        if( state.show_grid_colorbar)
        {
          bcbar=internal::BuildColorBar(bcells,1);
          bcbar->SetTitle("B    ");
          bcbar->SetLabelFormat(" %-2.0f     ");
          double brange[2];
          bface_lut->GetTableRange(brange);
          bcbar->SetNumberOfLabels((int)(brange[1]-brange[0]+1));
          Figure::RTAddActor2D(bcbar);
        }
      }
    }
  }
  
 
  /////////////////////////////////////////////////////////////////////
  /// 3D Filter

  template <class DATA,class FILTER>
  void GridView::RTBuildVTKPipeline3D()
  {
    RTCalcTransform();
    {
      double range[2];
      auto cr=vtkDoubleArray::SafeDownCast(DATA::SafeDownCast(data_producer->GetOutputDataObject(0))->GetCellData()->GetAbstractArray("cellregions"));

      auto scalar = vtkSmartPointer<vtkAssignAttribute>::New();
      if (cr)
      {
        scalar->Assign("cellregions",vtkDataSetAttributes::SCALARS,vtkAssignAttribute::CELL_DATA);
        scalar->SetInputConnection(data_producer->GetOutputPort());
        cr->GetRange(range);
        
        cell_lut->SetTableRange(range[0],range[1]);
        cell_lut->Modified();
      }
      
      
      planeX->SetOrigin(trans_center);
      planeY->SetOrigin(trans_center);
      planeZ->SetOrigin(trans_center);
      
      auto transgeometry=vtkSmartPointer<vtkTransformFilter>::New();
      transgeometry->SetTransform(transform);
      
      
      if (cr)
        transgeometry->SetInputConnection(scalar->GetOutputPort());
      else
        transgeometry->SetInputConnection(data_producer->GetOutputPort());
      
      
      
      
      cutgeometry->SetInputConnection(transgeometry->GetOutputPort());
      
      auto cutpolydata=vtkSmartPointer<vtkGeometryFilter>::New();
      cutpolydata->SetInputConnection(cutgeometry->GetOutputPort());
      
      auto  cells = vtkSmartPointer<vtkPolyDataMapper>::New();
      cells->SetInputConnection(cutpolydata->GetOutputPort());
      
      if (cr)
      {
        cells->UseLookupTableScalarRangeOn();
        cells->SetLookupTable(cell_lut);
      }
      else
        cells->ScalarVisibilityOff();
#ifdef VTK_HAS_MAPPER_IMMEDIATE_RENDERING_ON
      cells->ImmediateModeRenderingOn();
#endif
      cellplot = vtkSmartPointer<vtkActor>::New();
      cellplot->SetMapper(cells);
      if (!cr)
        cellplot->GetProperty()->SetColor(0.9,0.9,0.9);
      Figure::RTAddActor(cellplot);
      
      
      // Extract edges is slow, plot cell polydata with wireframe instead
      // auto edges= vtkSmartPointer<vtkExtractEdges>::New();
      // edges->SetInputConnection(cutgeometry->GetOutputPort());
      // auto  emapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      // emapper->SetInputConnection(edges->GetOutputPort());
      // emapper->ScalarVisibilityOff();
      // edgeplot = vtkSmartPointer<vtkActor>::New();
      // edgeplot->GetProperty()->SetColor(0,0,0);
      // edgeplot->SetMapper(emapper);
      // Figure::RTAddActor(edgeplot);
      

      auto  celledges = vtkSmartPointer<vtkPolyDataMapper>::New();
      celledges->SetInputConnection(cutpolydata->GetOutputPort());
      celledges->ScalarVisibilityOff();
#ifdef VTK_HAS_MAPPER_IMMEDIATE_RENDERING_ON
      celledges->ImmediateModeRenderingOn();
#endif
      auto celledgeplot = vtkSmartPointer<vtkActor>::New();
      celledgeplot->SetMapper(celledges);
      celledgeplot->GetProperty()->SetColor(0,0,0);
      celledgeplot->GetProperty()->SetRepresentationToWireframe();
      Figure::RTAddActor(celledgeplot);


      if ( cr)
      {
        cbar=internal::BuildColorBar(cells);
        cbar->SetLabelFormat(" %-2.0f     ");
        cbar->SetNumberOfLabels((int)(range[1]-range[0]+1));
        cbar->SetTitle("c    ");
        Figure::RTAddActor2D(cbar);
      }
    }

    auto boundary_data=vtkDataSet::SafeDownCast(boundary_data_producer->GetOutputDataObject(0));
    if (boundary_data)
    {
      double brange[2];
      auto bcr=vtkDoubleArray::SafeDownCast(boundary_data->GetCellData()->GetAbstractArray("boundarycellregions"));

      if (bcr)
      {
        auto bscalar = vtkSmartPointer<vtkAssignAttribute>::New();
        bscalar->Assign("boundarycellregions",vtkDataSetAttributes::SCALARS,vtkAssignAttribute::CELL_DATA);
        bscalar->SetInputDataObject(boundary_data);
        bcr->GetRange(brange);
        
        bface_lut->SetTableRange(brange[0],brange[1]);
        bface_lut->Modified();
        auto bgeometry=vtkSmartPointer<FILTER>::New();
        bgeometry->SetInputConnection(bscalar->GetOutputPort());
        
        auto transbgeometry=vtkSmartPointer<vtkTransformPolyDataFilter>::New();
        transbgeometry->SetInputConnection(bgeometry->GetOutputPort());
        transbgeometry->SetTransform(transform);
        

        bcutgeometry->SetInputConnection(transbgeometry->GetOutputPort());
      
        auto bcutpolydata=vtkSmartPointer<vtkGeometryFilter>::New();
        bcutpolydata->SetInputConnection(bcutgeometry->GetOutputPort());



        auto  bcells = vtkSmartPointer<vtkPolyDataMapper>::New();
        bcells->SetInputConnection(bcutpolydata->GetOutputPort());
        
        bcells->UseLookupTableScalarRangeOn();
        bcells->SetLookupTable(bface_lut);
        bcells->ScalarVisibilityOn();
#ifdef VTK_HAS_MAPPER_IMMEDIATE_RENDERING_ON
        bcells->ImmediateModeRenderingOn();
#endif
        bcellplot = vtkSmartPointer<vtkActor>::New();
        bcellplot->SetMapper(bcells);
        Figure::RTAddActor(bcellplot);
        

        auto bedges= vtkSmartPointer<vtkExtractEdges>::New();
        bedges->SetInputConnection(bcutgeometry->GetOutputPort());
        auto  bemapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        bemapper->SetInputConnection(bedges->GetOutputPort());
        bemapper->ScalarVisibilityOff();
        bedgeplot = vtkSmartPointer<vtkActor>::New();
        bedgeplot->GetProperty()->SetColor(0,0,0);
        bedgeplot->SetMapper(bemapper);
        Figure::RTAddActor(bedgeplot);

        
        bcbar=internal::BuildColorBar(bcells,1);
        bcbar->SetTitle("b    ");
        bcbar->SetLabelFormat(" %-2.0f     ");
        bcbar->SetNumberOfLabels((int)(brange[1]-brange[0]+1));
        Figure::RTAddActor2D(bcbar);
      }
    }
  } 
  
  
  /////////////////////////////////////////////////////////////////////
  /// Generic access to filter
  void  GridView::RTBuildVTKPipeline()
  {
    if (state.datatype==DataSet::DataType::UnstructuredGrid)
    {
      if (state.spacedim==2)
        this->RTBuildVTKPipeline2D<vtkUnstructuredGrid,vtkGeometryFilter>();
      else
        this->RTBuildVTKPipeline3D<vtkUnstructuredGrid,vtkGeometryFilter>(); 
      return;
    }
    else if (state.datatype==DataSet::DataType::RectilinearGrid)
    {
      if (state.spacedim==2)
        this->RTBuildVTKPipeline2D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>();
      else
        this->RTBuildVTKPipeline3D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>();
      return;
    }
  }
}
