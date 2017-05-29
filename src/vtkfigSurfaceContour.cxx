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



#include "vtkfigSurfaceContour.h"


namespace vtkfig
{



  /////////////////////////////////////////////////////////////////////
  /// Constructor
  SurfaceContour::SurfaceContour(): Figure()
  {
  }
  


  /////////////////////////////////////////////////////////////////////
  /// 2D Filter

  template <class DATA, class FILTER>
  void SurfaceContour::RTBuild2D(
    vtkSmartPointer<vtkRenderWindow> window,
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer,
    vtkSmartPointer<DATA> gridfunc)
  {
    gridfunc->GetBounds(bounds);
    renderer->GetActiveCamera()->SetParallelProjection(1);

    auto values=vtkFloatArray::SafeDownCast(gridfunc->GetPointData()->GetAbstractArray(dataname.c_str()));
    double range[2];
    values->GetRange(range);
    SetVMinMax(range[0],range[1]);



    auto scalar = vtkSmartPointer<vtkAssignAttribute>::New();
    scalar->Assign(dataname.c_str(),vtkDataSetAttributes::SCALARS,vtkAssignAttribute::POINT_DATA);
    scalar->SetInputDataObject(gridfunc);



    auto geometry=vtkSmartPointer<FILTER>::New();
    geometry->SetInputConnection(scalar->GetOutputPort());
    

    SetModelTransform(renderer,2,bounds);

        
    if (state.show_surface)
    {
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(geometry->GetOutputPort());
      
      mapper->UseLookupTableScalarRangeOn();
      mapper->SetLookupTable(surface_lut);
      mapper->ImmediateModeRenderingOn();
      
      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->SetMapper(mapper);
      Figure::RTAddActor(plot);
      
      if (state.show_surface_colorbar)
        Figure::RTAddActor2D(BuildColorBar(mapper));
    }
    
    
    if (state.show_isocontours)
    {
      
      isocontours->SetInputConnection(geometry->GetOutputPort());
      
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(isocontours->GetOutputPort());
      mapper->UseLookupTableScalarRangeOn();
      mapper->SetLookupTable(contour_lut);
      
      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->SetMapper(mapper);
      plot->GetProperty()->SetLineWidth(state.contour_line_width);
      Figure::RTAddActor(plot);
      if (state.show_contour_colorbar)
        Figure::RTAddActor2D(BuildColorBar(mapper));
      
      if (state.show_slider)
        AddSlider(interactor,renderer);
      
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

    Figure::RTAddAnnotations();  

  }
  
 
  /////////////////////////////////////////////////////////////////////
  /// 3D Filter

  template <class DATA,class FILTER>
  void SurfaceContour::RTBuild3D(
    vtkSmartPointer<vtkRenderWindow> window,
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer,
    vtkSmartPointer<DATA> gridfunc)
  {


    gridfunc->GetBounds(bounds);
    gridfunc->GetCenter(center);
    Figure::SetModelTransform(renderer,3,bounds);

    auto scalar = vtkSmartPointer<vtkAssignAttribute>::New();
    scalar->Assign(dataname.c_str(),vtkDataSetAttributes::SCALARS,vtkAssignAttribute::POINT_DATA);
    scalar->SetInputDataObject(gridfunc);





    planeX->SetOrigin(center);
    planeY->SetOrigin(center);
    planeZ->SetOrigin(center);
    
    planecutX->SetInputConnection(scalar->GetOutputPort());
    planecutX->SetCutFunction(planeX);
    planecutX->SetNumberOfContours(1);
    planecutX->SetValue(0,0.0);


    planecutY->SetInputConnection(scalar->GetOutputPort());
    planecutY->SetCutFunction(planeY);
    planecutY->SetNumberOfContours(1);
    planecutY->SetValue(0,0.0);
      
    planecutZ->SetInputConnection(scalar->GetOutputPort());
    planecutZ->SetCutFunction(planeZ);
    planecutZ->SetNumberOfContours(1);
    planecutZ->SetValue(0,0.0);
      

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

      
      if (state.show_slider)
        AddSlider(interactor,renderer);
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
    Figure::RTAddAnnotations();  
    
  } 
  
  
  /////////////////////////////////////////////////////////////////////
  /// Generic access to filter
  void  SurfaceContour::RTBuild(
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

  void SurfaceContour::ServerRTSend(vtkSmartPointer<Communicator> communicator)
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

  void SurfaceContour::ClientMTReceive(vtkSmartPointer<Communicator> communicator)
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
  }
  
}
