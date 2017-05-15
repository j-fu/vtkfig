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
    communicator->Send(gridfunc,1,1);
  }

  void TriContour2D::ClientMTReceive(vtkSmartPointer<Communicator> communicator) 
  {
    if (has_data)
    {
      gridfunc->Reset();
    }
    communicator->ReceiveCharBuffer((char*)&state,sizeof(state));
    communicator->Receive(gridfunc,1,1);
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

