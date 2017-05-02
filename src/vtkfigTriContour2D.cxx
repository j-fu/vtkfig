#include <vtkGeometryFilter.h>
#include <vtkContourFilter.h>
#include <vtkUnstructuredGridGeometryFilter.h>
#include "vtkActor.h"
#include "vtkfigTriContour2D.h"

namespace vtkfig
{
  
  void TriContour2D::Add(vtkSmartPointer<vtkUnstructuredGrid> gridfunc)
  {
    // filter to geometry primitive
    
    // see http://www.vtk.org/Wiki/VTK/Examples/Cxx/PolyData/GeometryFilter

    vtkSmartPointer<vtkGeometryFilter> geometry =  vtkSmartPointer<vtkGeometryFilter>::New();
    geometry->SetInputDataObject(gridfunc);
    double vrange[2];
    gridfunc->GetScalarRange(vrange);

    int lines=10;

    if (show_surface)
    {
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(geometry->GetOutputPort());
      mapper->SetScalarRange(vrange[0], vrange[1]);
      mapper->SetLookupTable(surface_lut);
      mapper->Update();

      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->SetMapper(mapper);
      Figure::AddActor(plot);
      
      if (show_surface_colorbar)
        Figure::AddActor(BuildColorBar(mapper));
    }


    if (show_contour)
    {
      vtkSmartPointer<vtkContourFilter> isocontours = vtkSmartPointer<vtkContourFilter>::New();
      isocontours->SetInputConnection(geometry->GetOutputPort());
      double tempdiff = (vrange[1]-vrange[0])/(10*lines);
      isocontours->GenerateValues(lines, vrange[0]+tempdiff, vrange[1]-tempdiff);


      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(isocontours->GetOutputPort());
      mapper->SetScalarRange(vrange[0], vrange[1]);
      mapper->SetLookupTable(contour_lut);

      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->SetMapper(mapper);
      Figure::AddActor(plot);
      if (show_contour_colorbar)
        Figure::AddActor(BuildColorBar(mapper));

    }

  }

  TriContour2D::TriContour2D(): vtkfig::Figure()
  {
    RGBTable surface_rgb={{0,0,0,1},{1,1,0,0}};
    RGBTable contour_rgb={{0,0,0,0},{1,0,0,0}};
    surface_lut=BuildLookupTable(surface_rgb,255);
    contour_lut=BuildLookupTable(contour_rgb,2);
  }
}

