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
#include "vtkCoordinate.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTransformFilter.h"
#include "vtkClipPolyData.h"
#include "vtkWarpScalar.h"

#include "vtkExtractCells.h"
#include "vtkIdList.h"


#include "vtkfigSurfaceContour.h"


namespace vtkfig
{



  /////////////////////////////////////////////////////////////////////
  /// Constructor
  SurfaceContour::SurfaceContour(): Figure()
  {
  }
  

  void SurfaceContour::SetSurfaceRGBTable(RGBTable & tab, int tabsize)
  {
    state.surface_rgbtab_size=tabsize;
    state.surface_rgbtab_modified=true;
    surface_rgbtab=tab;
    surface_lut=BuildLookupTable(tab,tabsize);
  }

  /////////////////////////////////////////////////////////////////////
  /// 2D Filter

  template <class DATA, class FILTER>
  void SurfaceContour::RTBuildVTKPipeline2D(vtkSmartPointer<DATA> gridfunc)
  {

    CalcTransform();

    /// should react on elevation view
    //renderer->GetActiveCamera()->SetParallelProjection(1);

    auto values=vtkFloatArray::SafeDownCast(gridfunc->GetPointData()->GetAbstractArray(dataname.c_str()));

    vtkSmartPointer<vtkExtractCells> subgrid;
    if (celllist)
    {
      subgrid=vtkSmartPointer<vtkExtractCells>::New();
      subgrid->SetInputDataObject(gridfunc);
      subgrid->SetCellList(celllist);
    }


    auto scalar = vtkSmartPointer<vtkAssignAttribute>::New();
    scalar->Assign(dataname.c_str(),vtkDataSetAttributes::SCALARS,vtkAssignAttribute::POINT_DATA);
    if (celllist)
      scalar->SetInputConnection(subgrid->GetOutputPort());
    else
      scalar->SetInputDataObject(gridfunc);

    auto geometry=vtkSmartPointer<FILTER>::New();
    geometry->SetInputConnection(scalar->GetOutputPort());



    auto transgeometry=vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transgeometry->SetInputConnection(geometry->GetOutputPort());
    transgeometry->SetTransform(transform);

    /// if we cut away a part of the ouline by vtkExtractCells, 
    /// we still need the axes to  be correct, so they 
    /// must be created using the original geometry
    vtkSmartPointer<vtkTransformPolyDataFilter> transallgeometry;
    if (celllist)
    {
      auto allgeometry=vtkSmartPointer<FILTER>::New();
      allgeometry->SetInputDataObject(gridfunc);

      transallgeometry=vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      transallgeometry->SetTransform(transform);
      transallgeometry->SetInputConnection(allgeometry->GetOutputPort());
    }
    else 
      transallgeometry=transgeometry;

    if (true) // Elevation
    {
      auto wtransform =  vtkSmartPointer<vtkTransform>::New();
      wtransform->Translate(0,0,0.5);
      auto wtransgeometry=vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      wtransgeometry->SetInputConnection(transgeometry->GetOutputPort());
      wtransgeometry->SetTransform(wtransform);

      auto elevation = vtkSmartPointer<vtkWarpScalar>::New();
      elevation->SetInputConnection(wtransgeometry->GetOutputPort());
      elevation->SetScaleFactor(0.5/(state.real_vmax-state.real_vmin));
      vtkSmartPointer<vtkPolyDataMapper> wmapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      wmapper->SetInputConnection(elevation->GetOutputPort());
      vtkSmartPointer<vtkActor> wplot = vtkSmartPointer<vtkActor>::New();
      wmapper->UseLookupTableScalarRangeOn();
      wmapper->SetLookupTable(elevation_lut);
      elevation_plot->SetMapper(wmapper);
      elevation_plot->SetVisibility(state.show_elevation);
      Figure::RTAddActor(elevation_plot);
    }


    if (true)
    {
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(transgeometry->GetOutputPort());

//      mapper->InterpolateScalarsBeforeMappingOn();
      mapper->UseLookupTableScalarRangeOn();
      mapper->SetLookupTable(surface_lut);
      mapper->ImmediateModeRenderingOn();

      surface_plot->SetMapper(mapper);
      surface_plot->SetVisibility(state.show_surface);
      Figure::RTAddActor(surface_plot);
      
      if (state.show_surface_colorbar)
        Figure::RTAddActor2D(BuildColorBar(mapper));
    }
    
    
    if (true)
    {
      
      isoline_filter->SetInputConnection(transgeometry->GetOutputPort());
      
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(isoline_filter->GetOutputPort());
      //mapper->UseLookupTableScalarRangeOn();
      //mapper->SetLookupTable(contour_lut);
      mapper->ScalarVisibilityOff();
      
      isoline_plot->SetMapper(mapper);
      isoline_plot->GetProperty()->SetColor(0,0,0);
      isoline_plot->GetProperty()->SetLineWidth(state.isoline_width);
      isoline_plot->SetVisibility(state.show_isolines);
      Figure::RTAddActor(isoline_plot);

      // if (state.show_contour_colorbar)
      //   Figure::RTAddActor2D(BuildColorBar(mapper));
      
      // if (state.show_slider)
      //   AddSlider(interactor,renderer);
      
    } 


  }
  
 
  /////////////////////////////////////////////////////////////////////
  /// 3D Filter

  template <class DATA,class FILTER>
  void SurfaceContour::RTBuildVTKPipeline3D(vtkSmartPointer<DATA> gridfunc)
  {
    CalcTransform();



    vtkSmartPointer<vtkExtractCells> subgrid;
    if (celllist)
    {
      subgrid=vtkSmartPointer<vtkExtractCells>::New();
      subgrid->SetInputDataObject(gridfunc);
      subgrid->SetCellList(celllist);
    }

    
    
    auto scalar = vtkSmartPointer<vtkAssignAttribute>::New();
    scalar->Assign(dataname.c_str(),vtkDataSetAttributes::SCALARS,vtkAssignAttribute::POINT_DATA);
    if (celllist)
      scalar->SetInputConnection(subgrid->GetOutputPort());
    else
      scalar->SetInputDataObject(gridfunc);
    



    auto transgeometry=vtkSmartPointer<vtkTransformFilter>::New();
    transgeometry->SetInputConnection(scalar->GetOutputPort());
    transgeometry->SetTransform(transform);

    vtkSmartPointer<vtkTransformFilter> transallgeometry;
    if (celllist)
    {
      auto allgeometry=vtkSmartPointer<FILTER>::New();
      allgeometry->SetInputDataObject(gridfunc);

      transallgeometry=vtkSmartPointer<vtkTransformFilter>::New();
      transallgeometry->SetTransform(transform);
      transallgeometry->SetInputConnection(allgeometry->GetOutputPort());
    }
    else 
      transallgeometry=transgeometry;




    planeX->SetOrigin(trans_center);
    planeY->SetOrigin(trans_center);
    planeZ->SetOrigin(trans_center);
    
    planecutX->SetInputConnection(transgeometry->GetOutputPort());
    planecutX->SetCutFunction(planeX);
    planecutX->SetNumberOfContours(0);


    planecutY->SetInputConnection(transgeometry->GetOutputPort());
    planecutY->SetCutFunction(planeY);
    planecutY->SetNumberOfContours(0);
      
    planecutZ->SetInputConnection(transgeometry->GetOutputPort());
    planecutZ->SetCutFunction(planeZ);
    planecutZ->SetNumberOfContours(1);
    planecutZ->SetValue(0,0.0);
      

    vtkSmartPointer<vtkClipPolyData> clipgeometry=0;
    if (false) // plot complete outline surface
    {
      
      clipgeometry=vtkSmartPointer<vtkClipPolyData>::New();
      auto sfilter=vtkSmartPointer<FILTER>::New();
      sfilter->SetInputConnection(transgeometry->GetOutputPort());
      clipgeometry->SetInputConnection(sfilter->GetOutputPort());
      clipgeometry->SetClipFunction(planeZ);
      clipgeometry->SetInsideOut(1);

      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();


      mapper->SetInputConnection(clipgeometry->GetOutputPort());
      mapper->UseLookupTableScalarRangeOn();
      mapper->SetLookupTable(surface_lut);
      auto splot=vtkSmartPointer<vtkActor>::New();
      splot->GetProperty()->SetOpacity(1.0);
      splot->SetMapper(mapper);
      Figure::RTAddActor(splot);
      
    }



    auto xyz =    vtkSmartPointer<vtkAppendPolyData>::New();
    xyz->SetUserManagedInputs(1);
    if (clipgeometry)
        xyz->SetNumberOfInputs(4);
    else
        xyz->SetNumberOfInputs(3);

    xyz->SetInputConnectionByNumber(0,planecutX->GetOutputPort());
    xyz->SetInputConnectionByNumber(1,planecutY->GetOutputPort());
    xyz->SetInputConnectionByNumber(2,planecutZ->GetOutputPort());
    if (clipgeometry)
      xyz->SetInputConnectionByNumber(3,clipgeometry->GetOutputPort());


    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(xyz->GetOutputPort());
    mapper->UseLookupTableScalarRangeOn();
    mapper->SetLookupTable(surface_lut);
    
    vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
    plot->GetProperty()->SetOpacity(1);
    plot->SetMapper(mapper);
    Figure::RTAddActor(plot);
    
    Figure::RTAddActor2D(BuildColorBar(mapper));

    
    
    if (true)
    {
      
      isoline_filter->SetInputConnection(xyz->GetOutputPort());
        
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(isoline_filter->GetOutputPort());
      // mapper->UseLookupTableScalarRangeOn();
      // mapper->SetLookupTable(contour_lut);
      mapper->ScalarVisibilityOff();
      
      isoline_plot->GetProperty()->SetOpacity(1.0);
      isoline_plot->GetProperty()->SetColor(0,0,0);
      isoline_plot->GetProperty()->SetLineWidth(state.isoline_width);
      isoline_plot->SetMapper(mapper);
      isoline_plot->SetVisibility(state.show_isolines);
      Figure::RTAddActor(isoline_plot);
    }


    if (true)
    {
      
      isosurface_filter->SetInputConnection(transgeometry->GetOutputPort());
        
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(isosurface_filter->GetOutputPort());
      mapper->UseLookupTableScalarRangeOn();
      mapper->SetLookupTable(surface_lut);
      
      isosurface_plot->GetProperty()->SetOpacity(0.3);
      
      isosurface_plot->SetMapper(mapper);
      isosurface_plot->SetVisibility(state.show_isosurfaces);
      Figure::RTAddActor(isosurface_plot);
      
    }

    
  } 
  
  
  /////////////////////////////////////////////////////////////////////
  /// Generic access to filter
  void  SurfaceContour::RTBuildVTKPipeline()
  {
    SetNumberOfIsocontours(state.num_contours);

    if (state.datatype==DataSet::DataType::UnstructuredGrid)
    {
      auto griddata=vtkUnstructuredGrid::SafeDownCast(data);
      
      if (state.spacedim==2)
        this->RTBuildVTKPipeline2D<vtkUnstructuredGrid,vtkGeometryFilter>(griddata);
      else
        this->RTBuildVTKPipeline3D<vtkUnstructuredGrid,vtkGeometryFilter>(griddata); 
    }
    else if (state.datatype==DataSet::DataType::RectilinearGrid)
    {
      auto griddata=vtkRectilinearGrid::SafeDownCast(data);
      
      
      if (state.spacedim==2)
        this->RTBuildVTKPipeline2D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>(griddata);
      else
        this->RTBuildVTKPipeline3D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>(griddata);
    }
  }
  
  


  /////////////////////////////////////////////////////////////////////
  /// Client-Server communication

  void SurfaceContour::ServerRTSend(vtkSmartPointer<internals::Communicator> communicator)
  {
    if (state.surface_rgbtab_modified)
    {
      SendRGBTable(communicator, surface_rgbtab);
      state.surface_rgbtab_modified=false;
    }
  }

  void SurfaceContour::ClientMTReceive(vtkSmartPointer<internals::Communicator> communicator)
  {
    
    if (state.surface_rgbtab_modified)
    {
      RGBTable new_rgbtab;
      ReceiveRGBTable(communicator, new_rgbtab);
      SetSurfaceRGBTable(new_rgbtab,state.surface_rgbtab_size);
    }


  }
  
}
