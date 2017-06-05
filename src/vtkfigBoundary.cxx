#include "vtkProperty2D.h"
#include "vtkAlgorithmOutput.h"
#include "vtkTextProperty.h"
#include "vtkRectilinearGrid.h"
#include "vtkGeometryFilter.h"
#include "vtkRectilinearGridGeometryFilter.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridGeometryFilter.h"
#include "vtkOutlineFilter.h"
#include "vtkCubeAxesActor2D.h"
#include "vtkTextActor.h"
#include "vtkCoordinate.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkTransformFilter.h"
#include "vtkCamera.h"

#include "vtkfigBoundary.h"


namespace vtkfig
{



  Boundary::Boundary(): Figure()
  {
  }
  


  /////////////////////////////////////////////////////////////////////
  /// 2D Filter

  template <class DATA, class FILTER>
  void Boundary::RTBuildVTKPipeline2D(
    vtkSmartPointer<vtkRenderWindow> window,
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer,
    vtkSmartPointer<DATA> gridfunc)
  {
    auto transform=CalcTransform(gridfunc);

    /// should react on elevation view
    renderer->GetActiveCamera()->SetParallelProjection(1);

    auto geometry=vtkSmartPointer<FILTER>::New();
    geometry->SetInputDataObject(gridfunc);
    auto transgeometry=vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transgeometry->SetInputDataObject(gridfunc);
    transgeometry->SetTransform(transform);


    if (true)
    {
      auto axes=vtkSmartPointer<vtkCubeAxesActor2D>::New();
      axes->SetRanges(data_bounds);
      axes->SetUseRanges(1);
      axes->SetInputConnection(transgeometry->GetOutputPort());
      axes->GetProperty()->SetColor(0, 0, 0);
      axes->SetFontFactor(1.25);
      axes->SetCornerOffset(0); 
      axes->SetNumberOfLabels(3); 

      axes->SetCamera(renderer->GetActiveCamera());
      axes->ZAxisVisibilityOff();
      axes->SetXLabel("");
      axes->SetYLabel("");

      auto textprop=axes->GetAxisLabelTextProperty();
      textprop->ItalicOff();
      textprop->SetFontFamilyToArial();
      textprop->SetColor(0,0,0);

      textprop=axes->GetAxisTitleTextProperty();
      textprop->ItalicOff();
      textprop->SetFontFamilyToArial();
      textprop->SetColor(0,0,0);

      Figure::RTAddActor2D(axes);
    }

    Figure::RTAddAnnotations();  

  }
  
 
  /////////////////////////////////////////////////////////////////////
  /// 3D Filter

  template <class DATA,class FILTER>
  void Boundary::RTBuildVTKPipeline3D(
    vtkSmartPointer<vtkRenderWindow> window,
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer,
    vtkSmartPointer<DATA> gridfunc)
  {

    auto transform=CalcTransform(gridfunc);

    auto geometry=vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
    geometry->SetInputDataObject(gridfunc);

    auto transgeometry=vtkSmartPointer<vtkTransformFilter>::New();
    transgeometry->SetInputConnection(geometry->GetOutputPort());
    transgeometry->SetTransform(transform);


    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();

    mapper->SetInputConnection(transgeometry->GetOutputPort());
    auto splot=vtkSmartPointer<vtkActor>::New();
    splot->GetProperty()->SetOpacity(0.1);
    splot->GetProperty()->SetColor(0.8,0.8,0.8);
    splot->SetMapper(mapper);
    Figure::RTAddActor(splot);
  


    
    if (true)
    {
      // create outline
      vtkSmartPointer<vtkOutlineFilter>outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
      outlinefilter->SetInputConnection(transgeometry->GetOutputPort());
      vtkSmartPointer<vtkPolyDataMapper> outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      outlineMapper->SetInputConnection(outlinefilter->GetOutputPort());
      vtkSmartPointer<vtkActor> outline = vtkSmartPointer<vtkActor>::New();
      outline->SetMapper(outlineMapper);
      outline->GetProperty()->SetColor(0, 0, 0);
      Figure::RTAddActor(outline);
    }

    if (true)
    {
      auto axes=vtkSmartPointer<vtkCubeAxesActor2D>::New();
      axes->SetRanges(data_bounds);
      axes->SetUseRanges(1);
      axes->SetInputConnection(transgeometry->GetOutputPort());
      axes->GetProperty()->SetColor(0, 0, 0);
      axes->SetFontFactor(1.0);
      axes->SetCornerOffset(0); 
      axes->SetNumberOfLabels(3); 
      axes->SetInertia(100);

      axes->SetCamera(renderer->GetActiveCamera());
      axes->SetXLabel("x");
      axes->SetYLabel("y");
      axes->SetZLabel("z");

      auto textprop=axes->GetAxisLabelTextProperty();
      textprop->ItalicOff();
      textprop->SetFontFamilyToArial();
      textprop->SetColor(0,0,0);

      textprop=axes->GetAxisTitleTextProperty();
      textprop->ItalicOff();
      textprop->SetFontFamilyToArial();
      textprop->SetColor(0,0,0);
      
      Figure::RTAddActor2D(axes);
    }
    Figure::RTAddAnnotations();  
    
  } 
  
  
  /////////////////////////////////////////////////////////////////////
  /// Generic access to filter
  void  Boundary::RTBuildVTKPipeline(
    vtkSmartPointer<vtkRenderWindow> window,
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer)
  {
    SetNumberOfIsocontours(state.num_contours);

    if (state.datatype==DataSet::DataType::UnstructuredGrid)
    {
      auto griddata=vtkUnstructuredGrid::SafeDownCast(data);
      
      if (state.spacedim==2)
        this->RTBuildVTKPipeline2D<vtkUnstructuredGrid,vtkGeometryFilter>(window, interactor,renderer,griddata);
      else
        this->RTBuildVTKPipeline3D<vtkUnstructuredGrid,vtkGeometryFilter>(window,interactor,renderer,griddata); 
    }
    else if (state.datatype==DataSet::DataType::RectilinearGrid)
    {
      auto griddata=vtkRectilinearGrid::SafeDownCast(data);
      
      
      if (state.spacedim==2)
        this->RTBuildVTKPipeline2D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>(window, interactor,renderer,griddata);
      else
        this->RTBuildVTKPipeline3D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>(window, interactor,renderer,griddata);
    }
  }
  
  


  /////////////////////////////////////////////////////////////////////
  /// Client-Server communication

  void Boundary::ServerRTSend(vtkSmartPointer<internals::Communicator> communicator)
  {
    communicator->SendCharBuffer((char*)&state,sizeof(state));
    communicator->SendString(dataname);

    if (state.surface_rgbtab_modified)
    {
      SendRGBTable(communicator, surface_rgbtab);
      state.surface_rgbtab_modified=false;
    }

    if (state.contour_rgbtab_modified)
    {
      SendRGBTable(communicator, contour_rgbtab);
      state.contour_rgbtab_modified=false;
    }
    communicator->Send(data,1,1);
  }

  void Boundary::ClientMTReceive(vtkSmartPointer<internals::Communicator> communicator)
  {

    communicator->ReceiveCharBuffer((char*)&state,sizeof(state));
    communicator->ReceiveString(dataname);

    if (state.surface_rgbtab_modified)
    {
      RGBTable new_rgbtab;
      ReceiveRGBTable(communicator, new_rgbtab);
      SetSurfaceRGBTable(new_rgbtab,state.surface_rgbtab_size);
    }

    if (state.contour_rgbtab_modified)
    {
      RGBTable new_rgbtab;
      ReceiveRGBTable(communicator, new_rgbtab);
      SetContourRGBTable(new_rgbtab,state.contour_rgbtab_size);
    }

    if (data==NULL)
    {
      if (state.datatype==DataSet::DataType::RectilinearGrid)
        data=vtkSmartPointer<vtkRectilinearGrid>::New();
      else if (state.datatype==DataSet::DataType::UnstructuredGrid)
        data=vtkSmartPointer<vtkUnstructuredGrid>::New();
    }
    communicator->Receive(data,1,1);
  }
  
}
