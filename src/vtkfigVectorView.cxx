#include <vtkActor.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
#include <vtkContourFilter.h>
#include <vtkOutlineFilter.h>
#include <vtkRectilinearGridGeometryFilter.h>
#include <vtkGeometryFilter.h>
#include <vtkGlyph3D.h>
#include <vtkRectilinearGrid.h>
#include <vtkGlyphSource2D.h>
#include <vtkAssignAttribute.h>
#include <vtkProbeFilter.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransformFilter.h>
#include <vtkStreamTracer.h>
#include <vtkRibbonFilter.h>
#include <vtkArrayCalculator.h>


#include "vtkfigVectorView.h"
#include "vtkfigTools.h"
#include "config.h"


namespace vtkfig
{


  VectorView::VectorView(): Figure()  {};
  
  std::shared_ptr<VectorView> VectorView::New()
  {
    struct make_shared_enabler : public VectorView {};
    return std::make_shared<make_shared_enabler>();
  }


  void VectorView::SetQuiverRGBTable(RGBTable & tab, int tabsize)
  {
    state.quiver_rgbtab_size=tabsize;
    state.quiver_rgbtab_modified=true;
    quiver_rgbtab=tab;
    quiver_lut=internal::BuildLookupTable(tab,tabsize);
  }

  void VectorView::SetStreamLineRGBTable(RGBTable & tab, int tabsize)
  {
    state.stream_rgbtab_size=tabsize;
    state.stream_rgbtab_modified=true;
    stream_rgbtab=tab;
    stream_lut=internal::BuildLookupTable(tab,tabsize);
  }



  void VectorView::SetQuiverGrid(int nx, int ny)
  {
    assert(state.spacedim==2);
    double bounds[6];
    auto data=vtkDataSet::SafeDownCast(data_producer->GetOutputDataObject(0));

    data->GetBounds(bounds);
    
    auto probePoints =  vtkSmartPointer<vtkPoints>::New();
    double dx=(bounds[1]-bounds[0])/((double)nx);
    double dy=(bounds[3]-bounds[2])/((double)ny);

    double x=bounds[0]+0.5*dx;
    for (int ix=0; ix<nx;ix++,x+=dx )
    {
      double  y=bounds[2]+0.5*dy;
      for ( int iy=0;iy<ny;iy++,y+=dy )
        probePoints->InsertNextPoint ( x, y, 0);
    }

    probePolyData =vtkSmartPointer<vtkPolyData>::New();
    probePolyData->SetPoints(probePoints);
  }
  
  void VectorView::SetQuiverGrid(int nx, int ny, int nz)
  {
    assert(state.spacedim==3);

    double bounds[6];
    auto data=vtkDataSet::SafeDownCast(data_producer->GetOutputDataObject(0));
    data->GetBounds(bounds);
    
    auto probePoints =  vtkSmartPointer<vtkPoints>::New();
    double dx=(bounds[1]-bounds[0])/((double)nx);
    double dy=(bounds[3]-bounds[2])/((double)ny);
    double dz=(bounds[5]-bounds[4])/((double)nz);
    
    double x=bounds[0]+0.5*dx;
    for (int ix=0; ix<nx;ix++,x+=dx )
    {
      double  y=bounds[2]+0.5*dy;
      for ( int iy=0;iy<ny;iy++,y+=dy )
      {
        double  z=bounds[4]+0.5*dz;
        for ( int iz=0;iz<nz;iz++,z+=dz )
          probePoints->InsertNextPoint ( x, y, z);
      }
    }
    
    probePolyData =vtkSmartPointer<vtkPolyData>::New();
    probePolyData->SetPoints(probePoints);
  }


  template <class DATA>
  void  VectorView::RTBuildVTKPipeline0()
  {
    RTCalcTransform();

   
    /// be careful here: transform filter transforms vectors as well
    /// so one has to transform first without assiging the vector atrribute
    /// an assign after transform.
    auto transgeometry=vtkSmartPointer<vtkTransformFilter>::New();
    transgeometry->SetInputConnection(data_producer->GetOutputPort());
    transgeometry->SetTransform(transform);

   
    auto vector = vtkSmartPointer<vtkAssignAttribute>::New();
    vector->Assign(dataname.c_str(),vtkDataSetAttributes::VECTORS,vtkAssignAttribute::POINT_DATA);
    vector->SetInputConnection(transgeometry->GetOutputPort());



    if (state.show_quiver)
    {
      
      if (!probePolyData)
      {
        throw std::runtime_error("Please dont't forget to set quiver points/grid");
      }
      auto transprobe=vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      transprobe->SetInputDataObject(probePolyData);
      transprobe->SetTransform(transform);
      
      
      
      auto probeFilter = vtkSmartPointer<vtkProbeFilter>::New();
      probeFilter->SetComputeTolerance(true);
      probeFilter->SetSourceConnection(vector->GetOutputPort());
      probeFilter->SetInputConnection(transprobe->GetOutputPort());
      probeFilter->PassPointArraysOn();


      auto glyph = vtkSmartPointer<vtkGlyph3D>::New();
      glyph->SetInputConnection(probeFilter->GetOutputPort());
      glyph->SetColorModeToColorByVector();
      glyph->SetScaleModeToScaleByVector();
      
      
      if (state.spacedim==2)
      {
        arrow3ds->SetTipResolution(8);
        arrow3ds->SetTipLength(0.35);
        arrow3ds->SetTipRadius(0.15);
        arrow3ds->SetShaftRadius(0.075);
        
        glyph->SetSourceConnection(arrow3d->GetOutputPort());
      }
      else
      {
        arrow3ds->SetTipResolution(16);
        arrow3ds->SetTipLength(0.3);
        arrow3ds->SetTipRadius(0.1);
        arrow3ds->SetShaftRadius(0.025);
        glyph->SetSourceConnection(arrow3d->GetOutputPort());
      }
      // map gridfunction
      auto  mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(glyph->GetOutputPort());
      mapper->SetLookupTable(quiver_lut);
      mapper->UseLookupTableScalarRangeOn();
      
      
      // create plot quiver actor
      vtkSmartPointer<vtkActor> quiver_actor = vtkSmartPointer<vtkActor>::New();
      quiver_actor->SetMapper(mapper);
      
      
      if (state.show_quiver_colorbar)
        Figure::RTAddActor2D(internal::BuildColorBar(mapper));
      
      // add actors to renderer
      Figure::RTAddActor(quiver_actor);
    }

    if (state.show_stream)
    {
      /// put this into dataset ? So we can immediately get magnitude?
      /// CreateMagnitude, CreateComponent(i)
      /// Also, we get color scaling by lut.
      auto calc= vtkSmartPointer<vtkArrayCalculator>::New();
      calc->SetInputConnection(vector->GetOutputPort());
      calc->AddVectorArrayName(dataname.c_str());

    std::string func="";
    func+="mag(";
    func+=dataname;
    func+="*1.0)";


      // std::string func="";
      // func+="mag(";
      // func+=dataname;
      // func+=")";

      calc->SetFunction(func.c_str());
      calc->SetResultArrayName((dataname+"magnitude").c_str());
      
      
      auto vecmag = vtkSmartPointer<vtkAssignAttribute>::New();
      vecmag->Assign((dataname+"magnitude").c_str(),vtkDataSetAttributes::SCALARS,vtkAssignAttribute::POINT_DATA);
      vecmag->SetInputConnection(calc->GetOutputPort());
      
      
      auto transseed=vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      transseed->SetInputDataObject(seedPolyData);
      transseed->SetTransform(transform);
      
      
      
      auto stream=vtkSmartPointer<vtkStreamTracer>::New();
      stream->SetInputConnection(vecmag->GetOutputPort());
      stream->SetSourceConnection(transseed->GetOutputPort());
      stream->SetIntegrationStepUnit(vtkStreamTracer::LENGTH_UNIT);
      stream->SetInterpolatorType(vtkStreamTracer::INTERPOLATOR_WITH_CELL_LOCATOR);
      stream->SetMaximumPropagation(state.stream_maximum_propagation);
      stream->SetInitialIntegrationStep(state.stream_initial_integration_step);
      stream->SetMaximumIntegrationStep(state.stream_maximum_integration_step);
      stream->SetIntegrationDirectionToBoth();
      stream->SetIntegratorTypeToRungeKutta4();
      stream->SetMaximumNumberOfSteps(state.stream_maximum_number_of_steps);


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
        //stream->SetSurfaceStreamlines(1);
        // works only with point locator
        stream->SetComputeVorticity(false);
      }
      else
      { 
        stream->SetComputeVorticity(true);
      }
      
      
      
      
      auto ribbon=vtkSmartPointer<vtkRibbonFilter>::New();
      ribbon->SetInputConnection(stream->GetOutputPort());
      ribbon->SetWidth(state.stream_ribbonwidth);
      //ribbon->SetVaryWidth(1);
      
      auto  mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(ribbon->GetOutputPort());
      mapper->ScalarVisibilityOn();
      mapper->SetLookupTable(stream_lut);
      mapper->UseLookupTableScalarRangeOn();
              
      // create plot quiver actor
      vtkSmartPointer<vtkActor> stream_actor = vtkSmartPointer<vtkActor>::New();
      stream_actor->SetMapper(mapper);
      
      
      
      // add actors to renderer
      Figure::RTAddActor(stream_actor);
      if (state.show_stream_colorbar)
        Figure::RTAddActor2D(internal::BuildColorBar(mapper));
      
    }

  }



  /////////////////////////////////////////////////////////////////////
  /// Generic access to filter
  void  VectorView::RTBuildVTKPipeline()
  {
    
    if (state.datatype==DataSet::DataType::UnstructuredGrid)
    {
      this->RTBuildVTKPipeline0<vtkUnstructuredGrid>();
    }
    else if (state.datatype==DataSet::DataType::RectilinearGrid)
    {
      this->RTBuildVTKPipeline0<vtkRectilinearGrid>();
    }
  }

  /////////////////////////////////////////////////////////////////////
  /// Client-Server communication

  void VectorView::ServerMPSend(vtkSmartPointer<internals::Communicator> communicator)
  {
        cout <<"ServerRTSend" << endl;
    communicator->Send(probePolyData,1,1);
    communicator->Send(seedPolyData,1,1);
  }

  void VectorView::ClientMPReceive(vtkSmartPointer<internals::Communicator> communicator)
  {
        cout <<"ClientMTReceive" << endl;


    if (probePolyData==NULL)
      probePolyData=vtkSmartPointer<vtkPolyData>::New();
    communicator->Receive( probePolyData,1,1);


    if (seedPolyData==NULL)
      seedPolyData=vtkSmartPointer<vtkPolyData>::New();
    communicator->Receive(seedPolyData,1,1);


  }


}

