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



#include "vtkfigGridView.h"


namespace vtkfig
{


  /////////////////////////////////////////////////////////////////////
  /// Constructor
  GridView::GridView(): Figure()
  {
    sliderWidget = vtkSmartPointer<vtkSliderWidget>::New();
    surface_lut=BuildLookupTable(grid_rgbtab,grid_rgbtab_size);
  }
  


  /////////////////////////////////////////////////////////////////////
  /// 2D Filter

  template <class DATA, class FILTER>
  void GridView::RTBuild2D(
    vtkSmartPointer<vtkRenderWindow> window,
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer,
    vtkSmartPointer<DATA> gridfunc)
  {
    double bounds[6];
    gridfunc->GetBounds(bounds);
    renderer->GetActiveCamera()->SetParallelProjection(1);
    double range[2];
    auto cr=vtkFloatArray::SafeDownCast(gridfunc->GetCellData()->GetAbstractArray("cellregions"));
    auto scalar = vtkSmartPointer<vtkAssignAttribute>::New();
    if (cr)
    {
      scalar->Assign("cellregions",vtkDataSetAttributes::SCALARS,vtkAssignAttribute::CELL_DATA);
      scalar->SetInputDataObject(gridfunc);
      cr->GetRange(range);
      SetVMinMax(range[0],range[1]);
    }
    
    auto geometry=vtkSmartPointer<FILTER>::New();
    if (cr)
      geometry->SetInputConnection(scalar->GetOutputPort());
    else
      geometry->SetInputDataObject(gridfunc);
    
    
    SetModelTransform(renderer,2,bounds);

    auto  cells = vtkSmartPointer<vtkPolyDataMapper>::New();
    cells->SetInputConnection(geometry->GetOutputPort());

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
    edges->SetInputConnection(geometry->GetOutputPort());
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

    if (true)
    {
      auto axes=vtkSmartPointer<vtkCubeAxesActor2D>::New();
      axes->SetInputConnection(geometry->GetOutputPort());
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
    
    if (true)
    {
      auto tactor= vtkSmartPointer<vtkCornerAnnotation>::New();
      tactor->SetText(7,title.c_str());
      tactor->SetMinimumFontSize(8);
      tactor->SetMaximumFontSize(16);
      auto textprop=tactor->GetTextProperty();
      textprop->ItalicOff();
      textprop->BoldOn();
      textprop->SetFontSize(8);
      textprop->SetFontFamilyToArial();
      textprop->SetColor(0,0,0);
      Figure::RTAddActor2D(tactor);
    }

  }
  
 
  /////////////////////////////////////////////////////////////////////
  /// 3D Filter

  template <class DATA,class FILTER>
  void GridView::RTBuild3D(
    vtkSmartPointer<vtkRenderWindow> window,
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer,
    vtkSmartPointer<DATA> gridfunc)
  {

    cout << "3D Gridview not yet implemented" << endl;
    exit(1);
    double bounds[6];
    gridfunc->GetBounds(bounds);
    double center[3];
    gridfunc->GetCenter(center);
    Figure::SetModelTransform(renderer,3,bounds);

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
    
    
    if (state.show_isocontours)
    {
      

      if (state.show_isocontours_on_cutplanes)
        isocontours->SetInputConnection(xyz->GetOutputPort());
      else
        isocontours->SetInputConnection(scalar->GetOutputPort());
      
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(isocontours->GetOutputPort());
      mapper->UseLookupTableScalarRangeOn();
      mapper->SetLookupTable(contour_lut);
      
      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      if (state.show_isocontours_on_cutplanes)
      {
        plot->GetProperty()->SetOpacity(1.0);
        plot->GetProperty()->SetLineWidth(state.contour_line_width);
      }
      else
        plot->GetProperty()->SetOpacity(0.4);


      plot->SetMapper(mapper);
      Figure::RTAddActor(plot);
    }



    
    if (true)
    {
      // create outline
      vtkSmartPointer<vtkOutlineFilter>outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
      outlinefilter->SetInputConnection(scalar->GetOutputPort());;
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
      axes->SetInputData(gridfunc);
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

    if (true)
    {
      auto tactor= vtkSmartPointer<vtkCornerAnnotation>::New();
      tactor->SetText(7,title.c_str());
      auto textprop=tactor->GetTextProperty();
      textprop->ItalicOff();
      textprop->BoldOff();
      textprop->SetFontSize(8);
      textprop->SetFontFamilyToArial();
      textprop->SetColor(0,0,0);
      Figure::RTAddActor2D(tactor);
    }

  } 
  
  
  /////////////////////////////////////////////////////////////////////
  /// Generic access to filter
  void  GridView::RTBuild(
    vtkSmartPointer<vtkRenderWindow> window,
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer)
  {
    SetNumberOfIsocontours(state.num_contours);

    if (state.datatype==Figure::DataType::UnstructuredGrid)
    {
      auto griddata=vtkUnstructuredGrid::SafeDownCast(data);
      
      if (state.spacedim==2)
        this->RTBuild2D<vtkUnstructuredGrid,vtkGeometryFilter>(window, interactor,renderer,griddata);
      else
        this->RTBuild3D<vtkUnstructuredGrid,vtkGeometryFilter>(window,interactor,renderer,griddata); 
    }
    else if (state.datatype==Figure::DataType::RectilinearGrid)
    {
      auto griddata=vtkRectilinearGrid::SafeDownCast(data);
      
      
      if (state.spacedim==2)
        this->RTBuild2D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>(window, interactor,renderer,griddata);
      else
        this->RTBuild3D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>(window, interactor,renderer,griddata);
    }
  }
  
  


  /////////////////////////////////////////////////////////////////////
  /// Client-Server communication

  void GridView::ServerRTSend(vtkSmartPointer<Communicator> communicator)
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

  void GridView::ClientMTReceive(vtkSmartPointer<Communicator> communicator)
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
      if (state.datatype==Figure::DataType::RectilinearGrid)
        data=vtkSmartPointer<vtkRectilinearGrid>::New();
      else if (state.datatype==Figure::DataType::UnstructuredGrid)
        data=vtkSmartPointer<vtkUnstructuredGrid>::New();
    }
    communicator->Receive(data,1,1);


    data->Modified();
    data->GetPointData()->GetScalars()->Modified();

  }
  
}
