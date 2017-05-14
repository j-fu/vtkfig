#include "vtkActor.h"
#include "vtkProperty.h"
#include "vtkTextProperty.h"
#include "vtkFloatArray.h"
#include "vtkContourFilter.h"
#include "vtkOutlineFilter.h"
#include "vtkPlane.h"
#include "vtkCutter.h"
#include "vtkPolyDataNormals.h"
#include "vtkfigContour3D.h"

namespace vtkfig
{
  

////////////////////////////////////////////////////////////
  Contour3D::Contour3D(): Figure()
  {
    RGBTable slice_rgb={{0,0,0,1},{1,1,0,0}};
    RGBTable contour_rgb={{0,0,0,0},{1,0,0,0}};
    slice_lut=BuildLookupTable(slice_rgb,255);
    contour_lut=BuildLookupTable(contour_rgb,2);
    isocontours = vtkSmartPointer<vtkContourFilter>::New();
  }
  
  // Must go into contour3dbase
  void Contour3D::RTSetInteractor(vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                                      vtkSmartPointer<vtkRenderer> renderer) 
  {
      imageplaneWidget->SetInteractor(interactor);
      imageplaneWidget->SetInputConnection(geometry->GetOutputPort());
      imageplaneWidget->TextureInterpolateOff();
      imageplaneWidget->TextureVisibilityOff();
      imageplaneWidget->SetKeyPressActivationValue('x');
      imageplaneWidget->On();
  };
  
  void Contour3D::RTBuild(vtkSmartPointer<vtkRenderWindow> window,
                          vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                          vtkSmartPointer<vtkRenderer> renderer)
  {
    


    vtkSmartPointer<vtkRectilinearGrid> gridfunc=vtkSmartPointer<vtkRectilinearGrid>::New();

    int nisocontours=10;

    // Create rectilinear grid
    gridfunc->SetDimensions(Nx, Ny, Nz);
    gridfunc->SetXCoordinates(xcoord);
    gridfunc->SetYCoordinates(ycoord);
    gridfunc->SetZCoordinates(zcoord);

    gridfunc->GetPointData()->SetScalars(values);

    // filter to geometry primitive
    geometry =  vtkSmartPointer<vtkRectilinearGridGeometryFilter>::New();
    geometry->SetInputDataObject(gridfunc);

    if (0&&show_slice)
    {

      vtkSmartPointer<vtkPlane> plane= vtkSmartPointer<vtkPlane>::New();
      plane->SetOrigin(gridfunc->GetCenter());
      plane->SetNormal(1,0,0);

      vtkSmartPointer<vtkCutter> planecut= vtkSmartPointer<vtkCutter>::New();
      planecut->SetInputDataObject(gridfunc);
      planecut->SetCutFunction(plane);

      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(planecut->GetOutputPort());
      mapper->UseLookupTableScalarRangeOn();
      mapper->SetLookupTable(contour_lut);
    
      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->SetMapper(mapper);
      Figure::RTAddActor(plot);
    
      if (show_slice_colorbar)
        Figure::RTAddActor2D(BuildColorBar(mapper));

    }


    if (show_contour)
    {

      isocontours->SetInputDataObject(gridfunc);

      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(isocontours->GetOutputPort());
      mapper->UseLookupTableScalarRangeOn();
      mapper->SetLookupTable(contour_lut);

      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->GetProperty()->SetOpacity(0.5);
      plot->SetMapper(mapper);
      Figure::RTAddActor(plot);
      if (show_contour_colorbar)
        Figure::RTAddActor2D(BuildColorBar(mapper));

    }

    // create outline
    vtkSmartPointer<vtkOutlineFilter>outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
    outlinefilter->SetInputConnection(geometry->GetOutputPort());
    vtkSmartPointer<vtkPolyDataMapper> outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    outlineMapper->SetInputConnection(outlinefilter->GetOutputPort());
    vtkSmartPointer<vtkActor> outline = vtkSmartPointer<vtkActor>::New();
    outline->SetMapper(outlineMapper);
    outline->GetProperty()->SetColor(0, 0, 0);
    Figure::RTAddActor(outline);

    imageplaneWidget= vtkSmartPointer<vtkImagePlaneWidget>::New();

  }

  
}



