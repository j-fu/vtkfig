#include "vtkActor.h"
#include "vtkPointData.h"
#include "vtkProperty.h"
#include "vtkTextProperty.h"
#include "vtkFloatArray.h"
#include "vtkContourFilter.h"
#include "vtkOutlineFilter.h"
#include "vtkRectilinearGridGeometryFilter.h"
#include "vtkGeometryFilter.h"
#include "vtkGlyph3D.h"
#include "vtkRectilinearGrid.h"
#include "vtkGlyphSource2D.h"
#include "vtkAssignAttribute.h"
#include "vtkProbeFilter.h"


#include "vtkfigQuiver.h"

namespace vtkfig
{
////////////////////////////////////////////////////////////


  ////////////////////////////////////////////////////////////
  Quiver::Quiver(): Figure()  
  {  
    RGBTable quiver_rgb={{0,0,0,0},{1,0,0,0}};
    lut=BuildLookupTable(quiver_rgb,2);
  }
  

  template <class DATA, class FILTER>
  void  Quiver::RTBuild2D(
        vtkSmartPointer<vtkRenderWindow> window,
        vtkSmartPointer<vtkRenderWindowInteractor> interactor,
        vtkSmartPointer<vtkRenderer> renderer,
        vtkSmartPointer<DATA> gridfunc)
  {
    

    double  bounds[6];
    gridfunc->GetBounds(bounds);
    Figure::SetModelTransform(renderer,2,bounds);
    
    auto vector = vtkSmartPointer<vtkAssignAttribute>::New();
    vector->Assign(dataname.c_str(),vtkDataSetAttributes::VECTORS,vtkAssignAttribute::POINT_DATA);
    vector->SetInputDataObject(gridfunc);

    // filter to geometry primitive
    auto geometry = vtkSmartPointer<FILTER>::New();
    geometry->SetInputConnection(vector->GetOutputPort());


    auto probePoints =  vtkSmartPointer<vtkPoints>::New();
    
    double dx=(bounds[1]-bounds[0])/( (int)state.qv_nx);
    double dy=(bounds[3]-bounds[2])/( (int)state.qv_ny);

    double x,y;
    x=bounds[0];
    for (int ix=0; ix<state.qv_nx;ix++,x+=dx )
    {
      y=bounds[2];
      for ( int iy=0;iy<state.qv_ny;iy++,y+=dy )
      {
        probePoints->InsertNextPoint ( x, y, 0);
      }
    }

    auto probePolyData =vtkSmartPointer<vtkPolyData>::New();
    probePolyData->SetPoints(probePoints);
    

    auto probeFilter = vtkSmartPointer<vtkProbeFilter>::New();
    probeFilter->SetSourceConnection(geometry->GetOutputPort());
    probeFilter->SetInputData(probePolyData);
    probeFilter->PassPointArraysOn();



    // make a vector glyph
    auto arrow = vtkSmartPointer<vtkGlyphSource2D>::New();
    arrow->SetGlyphTypeToArrow();
    arrow->SetScale(state.qv_arrow_scale);
    arrow->FilledOff();

    auto glyph = vtkSmartPointer<vtkGlyph3D>::New();
    glyph->SetInputConnection(probeFilter->GetOutputPort());
    glyph->SetSourceConnection(arrow->GetOutputPort());
    glyph->SetColorModeToColorByVector();
    glyph->SetScaleModeToScaleByVector();


    // map gridfunction
    auto  mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(glyph->GetOutputPort());
    mapper->SetLookupTable(lut);
    mapper->UseLookupTableScalarRangeOn();


    // create plot quiver actor
    vtkSmartPointer<vtkActor> quiver_actor = vtkSmartPointer<vtkActor>::New();
    quiver_actor->SetMapper(mapper);



    // create outline
    // vtkSmartPointer<vtkOutlineFilter>outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
    // outlinefilter->SetInputConnection(geometry->GetOutputPort());
    // vtkSmartPointer<vtkPolyDataMapper> outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    // outlineMapper->SetInputConnection(outlinefilter->GetOutputPort());
    // vtkSmartPointer<vtkActor> outline = vtkSmartPointer<vtkActor>::New();
    // outline->SetMapper(outlineMapper);
    // outline->GetProperty()->SetColor(0, 0, 0);
    // Figure::AddActor(outline);

    // add actors to renderer
    Figure::RTAddActor(quiver_actor);
    if (show_colorbar)
      Figure::RTAddActor2D(BuildColorBar(mapper));
  }



  /////////////////////////////////////////////////////////////////////
  /// Generic access to filter
  void  Quiver::RTBuild(
    vtkSmartPointer<vtkRenderWindow> window,
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer)
  {

    if (state.datatype==Figure::DataType::UnstructuredGrid)
    {
      auto griddata=vtkUnstructuredGrid::SafeDownCast(data);
      
      if (state.spacedim==2)
        this->RTBuild2D<vtkUnstructuredGrid,vtkGeometryFilter>(window, interactor,renderer,griddata);

      // else
      //   this->RTBuild3D<vtkUnstructuredGrid,vtkGeometryFilter>(window,interactor,renderer,griddata); 
    }
    else if (state.datatype==Figure::DataType::RectilinearGrid)
    {
      auto griddata=vtkRectilinearGrid::SafeDownCast(data);
      
      
      if (state.spacedim==2)
        this->RTBuild2D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>(window, interactor,renderer,griddata);
      // else
      //   this->RTBuild3D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>(window, interactor,renderer,griddata);
    }
  }



}

