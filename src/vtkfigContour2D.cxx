#include "vtkRectilinearGridGeometryFilter.h"
#include "vtkStructuredGridGeometryFilter.h"
#include "vtkStructuredGrid.h"
#include "vtkContourFilter.h"
#include "vtkOutlineFilter.h"
#include "vtkPointData.h"
#include "vtkActor.h"
#include "vtkCommand.h"
#include "vtkProperty2D.h"
#include "vtkTextProperty.h"
#include "vtkSliderRepresentation2D.h"

#include "vtkfigContour2D.h"


namespace vtkfig
{
  
  
  
////////////////////////////////////////////////////////////
  Contour2D::Contour2D(): Contour2DBase()
  {
    xcoord = vtkSmartPointer<vtkFloatArray>::New();
    ycoord =vtkSmartPointer<vtkFloatArray>::New();
    values = vtkSmartPointer<vtkFloatArray>::New();
    gridfunc=vtkSmartPointer<vtkRectilinearGrid>::New();
    gridfunc->SetXCoordinates(xcoord);
    gridfunc->SetYCoordinates(ycoord);
    gridfunc->GetPointData()->SetScalars(values);

  }
  

  void Contour2D::ServerRTSend(vtkSmartPointer<Communicator> communicator) 
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
      communicator->Send(values,1,1);
  }

  void Contour2D::ClientMTReceive(vtkSmartPointer<Communicator> communicator) 
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
//      gridfunc->Reset();
    }

    if (grid_changed)
      communicator->Receive(gridfunc,1,1);
    else
      communicator->Receive(values,1,1);



    SetVMinMax(state.real_vmin,state.real_vmax);

    gridfunc->Modified();


    has_data=true;
  }

  
  void Contour2D::RTBuild(
        vtkSmartPointer<vtkRenderWindow> window,
        vtkSmartPointer<vtkRenderWindowInteractor> interactor,
        vtkSmartPointer<vtkRenderer> renderer)
  {

    double bounds[6];
    gridfunc->GetBounds(bounds);

    // filter to geometry primitive
    vtkSmartPointer<vtkRectilinearGridGeometryFilter> geometry =  vtkSmartPointer<vtkRectilinearGridGeometryFilter>::New();
    geometry->SetInputDataObject(gridfunc);


    ProcessData(interactor,renderer,geometry,bounds);

  }

}
