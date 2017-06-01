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
#include "vtkTransformPolyDataFilter.h"

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
  void  Quiver::RTBuildVTKPipeline2D(
        vtkSmartPointer<vtkRenderWindow> window,
        vtkSmartPointer<vtkRenderWindowInteractor> interactor,
        vtkSmartPointer<vtkRenderer> renderer,
        vtkSmartPointer<DATA> gridfunc)
  {
    auto transform=CalcTransform(gridfunc);
    


    

    auto vector = vtkSmartPointer<vtkAssignAttribute>::New();
    vector->Assign(dataname.c_str(),vtkDataSetAttributes::VECTORS,vtkAssignAttribute::POINT_DATA);
    vector->SetInputDataObject(gridfunc);

    // filter to geometry primitive
    auto geometry = vtkSmartPointer<FILTER>::New();
    geometry->SetInputConnection(vector->GetOutputPort());

    auto transgeometry=vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transgeometry->SetInputConnection(geometry->GetOutputPort());
    transgeometry->SetTransform(transform);


    auto probePoints =  vtkSmartPointer<vtkPoints>::New();
    double dx=(trans_bounds[1]-trans_bounds[0])/( (int)state.qv_nx);
    double dy=(trans_bounds[3]-trans_bounds[2])/( (int)state.qv_ny);

    double x,y;
    x=trans_bounds[0];
    for (int ix=0; ix<state.qv_nx;ix++,x+=dx )
    {
      y=trans_bounds[2];
      for ( int iy=0;iy<state.qv_ny;iy++,y+=dy )
      {
        probePoints->InsertNextPoint ( x, y, 0);
      }
    }

    auto probePolyData =vtkSmartPointer<vtkPolyData>::New();
    probePolyData->SetPoints(probePoints);
    

    auto probeFilter = vtkSmartPointer<vtkProbeFilter>::New();
    probeFilter->SetSourceConnection(transgeometry->GetOutputPort());
    probeFilter->SetInputData(probePolyData);
    probeFilter->PassPointArraysOn();


    arrow->SetScale(state.qv_arrow_scale);

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
  void  Quiver::RTBuildVTKPipeline(
    vtkSmartPointer<vtkRenderWindow> window,
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer)
  {

    if (state.datatype==DataSet::DataType::UnstructuredGrid)
    {
      auto griddata=vtkUnstructuredGrid::SafeDownCast(data);
      
      if (state.spacedim==2)
        this->RTBuildVTKPipeline2D<vtkUnstructuredGrid,vtkGeometryFilter>(window, interactor,renderer,griddata);

      // else
      //   this->RTBuildVTKPipeline3D<vtkUnstructuredGrid,vtkGeometryFilter>(window,interactor,renderer,griddata); 
    }
    else if (state.datatype==DataSet::DataType::RectilinearGrid)
    {
      auto griddata=vtkRectilinearGrid::SafeDownCast(data);
      
      
      if (state.spacedim==2)
        this->RTBuildVTKPipeline2D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>(window, interactor,renderer,griddata);
      // else
      //   this->RTBuildVTKPipeline3D<vtkRectilinearGrid,vtkRectilinearGridGeometryFilter>(window, interactor,renderer,griddata);
    }
  }



}

