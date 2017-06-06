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

#include "vtkfigDomain.h"


namespace vtkfig
{



  Domain::Domain(): Figure()
  {
  }
  

  /// \todo add filter to extract boundary edges
  template <class DATA,class FILTER>
  void Domain::RTBuildVTKPipeline(
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

    /// if boundary cell color set, use this one!
    if (state.show_domain_boundary && state.spacedim==3)
    {
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(transgeometry->GetOutputPort());
      auto splot=vtkSmartPointer<vtkActor>::New();
      if (state.spacedim==3)
      {
        splot->GetProperty()->SetOpacity(state.domain_opacity);
        splot->GetProperty()->SetColor(state.domain_surface_color);
      }
      else // TODO add filter to extract boundary edges
      {
        splot->GetProperty()->SetOpacity(1.0);
        splot->GetProperty()->SetColor(0,0,0);
      }

      splot->SetMapper(mapper);
      Figure::RTAddActor(splot);
    }


    
    if (state.show_domain_box&& state.spacedim==3)
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

    if (state.show_domain_axes)
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
      
      if (state.spacedim==2)
      {
        axes->SetXLabel("");
        axes->SetYLabel("");
        axes->ZAxisVisibilityOff();
      }
      else
      {
        axes->SetXLabel("x");
        axes->SetYLabel("y");
        axes->SetZLabel("z");
      }
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
  void  Domain::RTBuildVTKPipeline(
    vtkSmartPointer<vtkRenderWindow> window,
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer)
  {
    SetNumberOfIsocontours(state.num_contours);

    if (state.datatype==DataSet::DataType::UnstructuredGrid)
    {
      auto griddata=vtkUnstructuredGrid::SafeDownCast(data);
      this->RTBuildVTKPipeline<vtkUnstructuredGrid,vtkGeometryFilter>(window,interactor,renderer,griddata); 
    }
    else if (state.datatype==DataSet::DataType::RectilinearGrid)
    {
      auto griddata=vtkRectilinearGrid::SafeDownCast(data);
      this->RTBuildVTKPipeline<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>(window, interactor,renderer,griddata);
    }
  }
  
  


  /////////////////////////////////////////////////////////////////////
  /// Client-Server communication

  void Domain::ServerRTSend(vtkSmartPointer<internals::Communicator> communicator)
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

  void Domain::ClientMTReceive(vtkSmartPointer<internals::Communicator> communicator)
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
