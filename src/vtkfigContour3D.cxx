#include "vtkActor.h"
#include "vtkProperty.h"
#include "vtkTextProperty.h"
#include "vtkFloatArray.h"
#include "vtkContourFilter.h"
#include "vtkOutlineFilter.h"
#include "vtkRectilinearGridGeometryFilter.h"
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
  }
  
  
  void Contour3D::Add(
    vtkSmartPointer<vtkFloatArray> xcoord ,
    vtkSmartPointer<vtkFloatArray> ycoord ,
    vtkSmartPointer<vtkFloatArray> zcoord ,
    vtkSmartPointer<vtkFloatArray> values
    )
  {
    
    if (!Figure::IsEmpty())
      throw std::runtime_error("Contor3D already has data");


    vtkSmartPointer<vtkRectilinearGrid> gridfunc=vtkSmartPointer<vtkRectilinearGrid>::New();

    int Nx = xcoord->GetNumberOfTuples();
    int Ny = ycoord->GetNumberOfTuples();
    int Nz = zcoord->GetNumberOfTuples();

    int nisocontours=10;

    // Create rectilinear grid
    gridfunc->SetDimensions(Nx, Ny, Nz);
    gridfunc->SetXCoordinates(xcoord);
    gridfunc->SetYCoordinates(ycoord);
    gridfunc->SetZCoordinates(zcoord);

    gridfunc->GetPointData()->SetScalars(values);
    double vrange[2];
    gridfunc->GetScalarRange(vrange);

    if (show_slice)
    {

      vtkSmartPointer<vtkPlane> plane= vtkSmartPointer<vtkPlane>::New();
      plane->SetOrigin(gridfunc->GetCenter());
      plane->SetNormal(1,0,0);

      vtkSmartPointer<vtkCutter> planecut= vtkSmartPointer<vtkCutter>::New();
      planecut->SetInputDataObject(gridfunc);
      planecut->SetCutFunction(plane);


      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();

      mapper->SetInputConnection(planecut->GetOutputPort());
      mapper->SetScalarRange(vrange[0], vrange[1]);
      mapper->SetLookupTable(contour_lut);
    
      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->SetMapper(mapper);
      Figure::AddActor(plot);
    
      if (show_slice_colorbar)
        Figure::AddActor(Figure::BuildColorBar(mapper));

    }


    if (show_contour)
    {
      vtkSmartPointer<vtkContourFilter> isocontours = vtkSmartPointer<vtkContourFilter>::New();
      isocontours->SetInputData(gridfunc);

      double tempdiff = (vrange[1]-vrange[0])/(10*nisocontours);
      isocontours->GenerateValues(nisocontours, vrange[0]+tempdiff, vrange[1]-tempdiff);


      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(isocontours->GetOutputPort());
      mapper->SetScalarRange(vrange[0], vrange[1]);
      mapper->SetLookupTable(contour_lut);

      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->GetProperty()->SetOpacity(0.5);
      plot->SetMapper(mapper);
      Figure::AddActor(plot);
      if (show_contour_colorbar)
        Figure::AddActor(Figure::BuildColorBar(mapper));

    }

    // create outline
    vtkSmartPointer<vtkOutlineFilter>outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
    outlinefilter->SetInputData(gridfunc);
    vtkSmartPointer<vtkPolyDataMapper> outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    outlineMapper->SetInputConnection(outlinefilter->GetOutputPort());
    vtkSmartPointer<vtkActor> outline = vtkSmartPointer<vtkActor>::New();
    outline->SetMapper(outlineMapper);
    outline->GetProperty()->SetColor(0, 0, 0);
    Figure::AddActor(outline);

  }

  
}



