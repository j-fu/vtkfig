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
#include "vtkTransformFilter.h"


#include "vtkfigVectorView.h"


namespace vtkfig
{


  VectorView::VectorView(): Figure()  
  {  
    RGBTable quiver_rgb={{0,0,0,0},{1,0,0,0}};
    lut=BuildLookupTable(quiver_rgb,2);
  }
  


  void VectorView::SetQuiverGrid(int nx, int ny)
  {
    assert(data);
    assert(state.spacedim==2);
    double bounds[6];
    data->GetBounds(bounds);
    
    auto probePoints =  vtkSmartPointer<vtkPoints>::New();
    double dx=(bounds[1]-bounds[0])/((double)nx);
    double dy=(bounds[3]-bounds[2])/((double)ny);
    
    double x=bounds[0]+0.5*dx;
    for (int ix=0; ix<nx;ix++,x+=dx )
    {
      double  y=bounds[2]+0.5*dy;
      for ( int iy=0;iy<ny;iy++,y+=dy )
        probePoints->InsertNextPoint ( x, y, state.quiver_surface_distance);
    }

    probePolyData =vtkSmartPointer<vtkPolyData>::New();
    probePolyData->SetPoints(probePoints);
  }
  
  void VectorView::SetQuiverGrid(int nx, int ny, int nz)
  {
    assert(data);
    assert(state.spacedim==3);
    double bounds[6];
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
        {
          probePoints->InsertNextPoint ( x, y, z);
          // cout <<  x << " " << y << " " << z << endl;
        }
      }
    }
    
    probePolyData =vtkSmartPointer<vtkPolyData>::New();
    probePolyData->SetPoints(probePoints);
  }


  template <class DATA>
  void  VectorView::RTBuildVTKPipeline(vtkSmartPointer<DATA> gridfunc)
  {
    CalcTransform();

   
    /// be careful here: transform filter transforms vectors as well
    /// so one has to transform first without assiging the vector atrribute
    /// an assign after transform.
    auto transgeometry=vtkSmartPointer<vtkTransformFilter>::New();
    transgeometry->SetInputData(gridfunc);
    transgeometry->SetTransform(transform);

   
    auto vector = vtkSmartPointer<vtkAssignAttribute>::New();
    vector->Assign(dataname.c_str(),vtkDataSetAttributes::VECTORS,vtkAssignAttribute::POINT_DATA);
    vector->SetInputConnection(transgeometry->GetOutputPort());


    if (!probePolyData)
    {
      throw std::runtime_error("Please dont't forget to set quiver points/grid");
    }
    auto transprobe=vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transprobe->SetInputDataObject(probePolyData);
    transprobe->SetTransform(transform);

    auto probeFilter = vtkSmartPointer<vtkProbeFilter>::New();
    probeFilter->SetSourceConnection(vector->GetOutputPort());
    probeFilter->SetInputConnection(transprobe->GetOutputPort());
    probeFilter->PassPointArraysOn();

    

    auto glyph = vtkSmartPointer<vtkGlyph3D>::New();
    glyph->SetInputConnection(probeFilter->GetOutputPort());
//    glyph->SetColorModeToColorByVector();
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
      glyph->SetSourceConnection(arrow3d->GetOutputPort());
      arrow3ds->SetTipResolution(16);
      arrow3ds->SetTipLength(0.3);
      arrow3ds->SetTipRadius(0.1);
      arrow3ds->SetShaftRadius(0.025);
    }
    // map gridfunction
    auto  mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(glyph->GetOutputPort());
    // mapper->SetLookupTable(lut);
    // mapper->UseLookupTableScalarRangeOn();


    // create plot quiver actor
    vtkSmartPointer<vtkActor> quiver_actor = vtkSmartPointer<vtkActor>::New();
    if (state.spacedim==3)
      quiver_actor->GetProperty()->SetColor(0.5,0.5,0.5);
    else
      quiver_actor->GetProperty()->SetColor(0,0,0);
    quiver_actor->SetMapper(mapper);


    // add actors to renderer
    Figure::RTAddActor(quiver_actor);
  }



  /////////////////////////////////////////////////////////////////////
  /// Generic access to filter
  void  VectorView::RTBuildVTKPipeline()
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

  /////////////////////////////////////////////////////////////////////
  /// Client-Server communication

  void VectorView::ServerRTSend(vtkSmartPointer<internals::Communicator> communicator)
  {
    communicator->Send(probePolyData,1,1);
  }

  void VectorView::ClientMTReceive(vtkSmartPointer<internals::Communicator> communicator)
  {

    if (probePolyData==NULL)
      probePolyData=vtkSmartPointer<vtkPolyData>::New();

    communicator->Receive( probePolyData,1,1);

  }


}

