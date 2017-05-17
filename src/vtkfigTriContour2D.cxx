#include <vtkGeometryFilter.h>
#include <vtkContourFilter.h>
#include <vtkUnstructuredGridGeometryFilter.h>
#include "vtkActor.h"
#include "vtkfigTriContour2D.h"

namespace vtkfig
{
  TriContour2D::TriContour2D(): Contour2DBase()
  {
    gridfunc=vtkSmartPointer<vtkUnstructuredGrid>::New();
    gridvalues = vtkSmartPointer<vtkFloatArray>::New();
    gridpoints = vtkSmartPointer<vtkPoints>::New();
    gridfunc->SetPoints(gridpoints);
    gridfunc->GetPointData()->SetScalars(gridvalues);
  }



  void TriContour2D::ServerRTSend(vtkSmartPointer<Communicator> communicator) 
  {
    communicator->SendCharBuffer((char*)&state,sizeof(state));
    
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


    communicator->SendInt(grid_changed);
    if (grid_changed)
      communicator->Send(gridfunc,1,1);
    else
      communicator->Send(gridvalues,1,1);
  }

  void TriContour2D::ClientMTReceive(vtkSmartPointer<Communicator> communicator) 
  {

    communicator->ReceiveCharBuffer((char*)&state,sizeof(state));

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

    communicator->ReceiveInt(grid_changed);
    if (has_data|| grid_changed)
    {
      gridfunc->Reset();
    }

    if (grid_changed)
      communicator->Receive(gridfunc,1,1);
    else
      communicator->Receive(gridvalues,1,1);



    SetVMinMax(state.real_vmin,state.real_vmax);

    gridfunc->Modified();


    has_data=true;
  }

  
  void TriContour2D::RTBuild(
    vtkSmartPointer<vtkRenderWindow> window,
    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
    vtkSmartPointer<vtkRenderer> renderer)
  {
    
    vtkSmartPointer<vtkGeometryFilter> geometry =  vtkSmartPointer<vtkGeometryFilter>::New();
    geometry->SetInputDataObject(gridfunc);
    
    double bounds[6];
    gridfunc->GetBounds(bounds);
    ProcessData(interactor,renderer,geometry,bounds);
  }

}

