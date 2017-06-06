#include "vtkActor.h"
#include "vtkPointData.h"
#include "vtkProperty.h"
#include "vtkTextProperty.h"
#include "vtkFloatArray.h"
#include "vtkContourFilter.h"
#include "vtkOutlineFilter.h"
#include "vtkRectilinearGridGeometryFilter.h"
#include "vtkGeometryFilter.h"
#include "vtkRectilinearGrid.h"
#include "vtkAssignAttribute.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTransformFilter.h"
#include "vtkStreamTracer.h"
#include "vtkRibbonFilter.h"


#include "vtkfigStream.h"

namespace vtkfig
{


  Stream::Stream(): Figure()  
  {  
    RGBTable quiver_rgb={{0,0,0,0},{1,0,0,0}};
    lut=BuildLookupTable(quiver_rgb,2);
  }
  



  template <class DATA>
  void  Stream::RTBuildVTKPipeline(vtkSmartPointer<DATA> gridfunc)
  {
    auto transform=CalcTransform(gridfunc);
    

    auto vector = vtkSmartPointer<vtkAssignAttribute>::New();
    vector->Assign(dataname.c_str(),vtkDataSetAttributes::VECTORS,vtkAssignAttribute::POINT_DATA);
    vector->SetInputDataObject(gridfunc);
    
    auto transgeometry=vtkSmartPointer<vtkTransformFilter>::New();
    transgeometry->SetInputConnection(vector->GetOutputPort());
    transgeometry->SetTransform(transform);


    auto transprobe=vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transprobe->SetInputDataObject(probePolyData);
    transprobe->SetTransform(transform);



    auto stream=vtkSmartPointer<vtkStreamTracer>::New();
    stream->SetInputConnection(transgeometry->GetOutputPort());
    stream->SetSourceConnection(transprobe->GetOutputPort());
    
    stream->SetMaximumPropagation(state.streamlength);
    stream->SetInitialIntegrationStep(.2);
    stream->SetIntegrationDirectionToForward();
    stream->SetIntegratorTypeToRungeKutta45();
    //stream->VorticityOn();
    
// https://github.com/vejmarie/vtk-7/blob/master/Examples/GUI/Python/StreamlinesWithLineWidget.py
// streamer = vtk.vtkStreamTracer()
// streamer.SetInputData(pl3d_output)
// streamer.SetSourceData(seeds)
// streamer.SetMaximumPropagation(100)
// streamer.SetInitialIntegrationStep(.2)
// streamer.SetIntegrationDirectionToForward()
// streamer.SetComputeVorticity(1)
// streamer.SetIntegrator(rk4)
// rf = vtk.vtkRibbonFilter()
// rf.SetInputConnection(streamer.GetOutputPort())
// rf.SetWidth(0.1)
// rf.SetWidthFactor(5)
// streamMapper = vtk.vtkPolyDataMapper()
// streamMapper.SetInputConnection(rf.GetOutputPort())
// streamMapper.SetScalarRange(pl3d_output.GetScalarRange())
// streamline = vtk.vtkActor()
// streamline.SetMapper(streamMapper)


    if (state.spacedim==2)
    {    
      stream->SetSurfaceStreamlines(1);
      stream->SetComputeVorticity(false);
    }
    else
    { 
      stream->SetComputeVorticity(true);
    }


    auto ribbon=vtkSmartPointer<vtkRibbonFilter>::New();
    ribbon->SetInputConnection(stream->GetOutputPort());
    ribbon->SetWidth(state.streamribbonwidth);

    // map gridfunction
    auto  mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(ribbon->GetOutputPort());
    // mapper->SetLookupTable(lut);
    // mapper->UseLookupTableScalarRangeOn();


    // create plot quiver actor
    vtkSmartPointer<vtkActor> stream_actor = vtkSmartPointer<vtkActor>::New();
    stream_actor->GetProperty()->SetColor(state.streamcolor);
    stream_actor->SetMapper(mapper);



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
    Figure::RTAddActor(stream_actor);
  }



  /////////////////////////////////////////////////////////////////////
  /// Generic access to filter
  void  Stream::RTBuildVTKPipeline()
  {

    if (state.datatype==DataSet::DataType::UnstructuredGrid)
    {
      auto griddata=vtkUnstructuredGrid::SafeDownCast(data);
      this->RTBuildVTKPipeline<vtkUnstructuredGrid>(griddata);
    }
    else if (state.datatype==DataSet::DataType::RectilinearGrid)
    {
      auto griddata=vtkRectilinearGrid::SafeDownCast(data);
      this->RTBuildVTKPipeline<vtkRectilinearGrid>(griddata);
    }
  }



}

