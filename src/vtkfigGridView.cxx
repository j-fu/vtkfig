#include "vtkSliderRepresentation2D.h"
#include "vtkProperty2D.h"
#include "vtkAlgorithmOutput.h"
#include "vtkTextProperty.h"
#include "vtkRectilinearGrid.h"
#include "vtkGeometryFilter.h"
#include "vtkRectilinearGridGeometryFilter.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridGeometryFilter.h"
#include "vtkPlane.h"
#include "vtkCutter.h"
#include "vtkImplicitBoolean.h"
#include "vtkOutlineFilter.h"
#include "vtkCubeAxesActor2D.h"
#include "vtkAppendPolyData.h"
#include "vtkAssignAttribute.h"
#include "vtkCamera.h"
#include "vtkTextActor.h"
#include "vtkCornerAnnotation.h"
#include "vtkCoordinate.h"
#include "vtkExtractEdges.h"
#include "vtkTransformPolyDataFilter.h"



#include "vtkfigGridView.h"


namespace vtkfig
{


  /////////////////////////////////////////////////////////////////////
  /// Constructor
  GridView::GridView(): Figure()
  {
    surface_lut=BuildLookupTable(grid_rgbtab,grid_rgbtab_size);
  }
  


  /////////////////////////////////////////////////////////////////////
  /// 2D Filter

  template <class DATA, class FILTER>
  void GridView::RTBuildVTKPipeline2D(vtkSmartPointer<DATA> gridfunc)
  {
    CalcTransform();

    double range[2];
    auto cr=vtkFloatArray::SafeDownCast(gridfunc->GetCellData()->GetAbstractArray("cellregions"));
    auto scalar = vtkSmartPointer<vtkAssignAttribute>::New();
    if (cr)
    {
      scalar->Assign("cellregions",vtkDataSetAttributes::SCALARS,vtkAssignAttribute::CELL_DATA);
      scalar->SetInputDataObject(gridfunc);
      cr->GetRange(range);

      surface_lut->SetTableRange(range[0],range[1]);
      surface_lut->Modified();
    }
    
    auto geometry=vtkSmartPointer<FILTER>::New();
    if (cr)
      geometry->SetInputConnection(scalar->GetOutputPort());
    else
      geometry->SetInputDataObject(gridfunc);
    
    

    auto transgeometry=vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transgeometry->SetInputConnection(geometry->GetOutputPort());
    transgeometry->SetTransform(transform);

    auto  cells = vtkSmartPointer<vtkPolyDataMapper>::New();
    cells->SetInputConnection(transgeometry->GetOutputPort());

    if (cr)
    {
      cells->UseLookupTableScalarRangeOn();
      cells->SetLookupTable(surface_lut);
    }
    else
      cells->ScalarVisibilityOff();
    cells->ImmediateModeRenderingOn();
    auto    cellplot = vtkSmartPointer<vtkActor>::New();
    cellplot->SetMapper(cells);
    if (!cr)
      cellplot->GetProperty()->SetColor(0.9,0.9,0.9);
    Figure::RTAddActor(cellplot);

    auto edges= vtkSmartPointer<vtkExtractEdges>::New();
    edges->SetInputConnection(transgeometry->GetOutputPort());
    auto  emapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    emapper->SetInputConnection(edges->GetOutputPort());
    emapper->ScalarVisibilityOff();
    auto    edgeplot = vtkSmartPointer<vtkActor>::New();
    edgeplot->GetProperty()->SetColor(0,0,0);
    edgeplot->SetMapper(emapper);
    Figure::RTAddActor(edgeplot);


      
    if (state.show_surface_colorbar && cr)
    {
      auto cbar=BuildColorBar(cells);
      cbar->SetLabelFormat(" %-2.0f     ");
      cbar->SetNumberOfLabels((int)(range[1]-range[0]+1));
      Figure::RTAddActor2D(cbar);
    }

    

  }
  
 
  /////////////////////////////////////////////////////////////////////
  /// 3D Filter

  template <class DATA,class FILTER>
  void GridView::RTBuildVTKPipeline3D(vtkSmartPointer<DATA> gridfunc)
  {
    CalcTransform();

    cout << "3D Gridview not yet implemented" << endl;
    exit(1);
    double bounds[6];
    gridfunc->GetBounds(bounds);
    double center[3];
    gridfunc->GetCenter(center);


    auto scalar = vtkSmartPointer<vtkAssignAttribute>::New();
    scalar->Assign(dataname.c_str(),vtkDataSetAttributes::SCALARS,vtkAssignAttribute::POINT_DATA);
    scalar->SetInputDataObject(gridfunc);




    auto planeX= vtkSmartPointer<vtkPlane>::New();
    planeX->SetOrigin(center);
    planeX->SetNormal(1,0,0);
      
    auto planeY= vtkSmartPointer<vtkPlane>::New();
    planeY->SetOrigin(center);
    planeY->SetNormal(0,1,0);
      
    auto planeZ= vtkSmartPointer<vtkPlane>::New();
    planeZ->SetOrigin(center);
    planeZ->SetNormal(0,0,1);
    
    
    auto planecutX= vtkSmartPointer<vtkCutter>::New();
    planecutX->SetInputConnection(scalar->GetOutputPort());
    planecutX->SetCutFunction(planeX);

      
    auto planecutY= vtkSmartPointer<vtkCutter>::New();
    planecutY->SetInputConnection(scalar->GetOutputPort());
    planecutY->SetCutFunction(planeY);
      
    auto planecutZ= vtkSmartPointer<vtkCutter>::New();
    planecutZ->SetInputConnection(scalar->GetOutputPort());
    planecutZ->SetCutFunction(planeZ);
      

    auto xyz =    vtkSmartPointer<vtkAppendPolyData>::New();
    xyz->SetUserManagedInputs(1);
    xyz->SetNumberOfInputs(3);

    xyz->SetInputConnectionByNumber(0,planecutX->GetOutputPort());
    xyz->SetInputConnectionByNumber(1,planecutY->GetOutputPort());
    xyz->SetInputConnectionByNumber(2,planecutZ->GetOutputPort());


    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(xyz->GetOutputPort());
    mapper->UseLookupTableScalarRangeOn();
    mapper->SetLookupTable(surface_lut);
    
    vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
    plot->GetProperty()->SetOpacity(1);
    plot->SetMapper(mapper);
    Figure::RTAddActor(plot);
    
    Figure::RTAddActor2D(BuildColorBar(mapper));
    
  } 
  
  
  /////////////////////////////////////////////////////////////////////
  /// Generic access to filter
  void  GridView::RTBuildVTKPipeline()
  {


    auto udata=vtkUnstructuredGrid::SafeDownCast(data);
    if (udata)
    {
      
      if (state.spacedim==2)
        this->RTBuildVTKPipeline2D<vtkUnstructuredGrid,vtkGeometryFilter>(udata);
      else
        this->RTBuildVTKPipeline3D<vtkUnstructuredGrid,vtkGeometryFilter>(udata); 
      return;
    }

    auto rdata=vtkRectilinearGrid::SafeDownCast(data);
    if (rdata)
    {
      if (state.spacedim==2)
        this->RTBuildVTKPipeline2D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>(rdata);
      else
        this->RTBuildVTKPipeline3D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>(rdata);
      return;
    }
  }
  
  


  
}
