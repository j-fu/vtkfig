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

  }
  
  
  void Contour2D::RTBuild(
        vtkSmartPointer<vtkRenderWindow> window,
        vtkSmartPointer<vtkRenderWindowInteractor> interactor,
        vtkSmartPointer<vtkRenderer> renderer)
  {

    vtkSmartPointer<vtkRectilinearGrid> gridfunc=vtkSmartPointer<vtkRectilinearGrid>::New();
    int Nx = xcoord->GetNumberOfTuples();
    int Ny = ycoord->GetNumberOfTuples();

    // Create rectilinear grid
    gridfunc->SetDimensions(Nx, Ny, 1);
    gridfunc->SetXCoordinates(xcoord);
    gridfunc->SetYCoordinates(ycoord);
    gridfunc->GetPointData()->SetScalars(values);

    double bounds[6];
    gridfunc->GetBounds(bounds);

    // filter to geometry primitive
    vtkSmartPointer<vtkRectilinearGridGeometryFilter> geometry =  vtkSmartPointer<vtkRectilinearGridGeometryFilter>::New();
    geometry->SetInputDataObject(gridfunc);


    ProcessData(interactor,renderer,geometry,bounds);

  }

}
