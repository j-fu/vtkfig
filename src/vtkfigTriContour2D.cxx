#include <vtkGeometryFilter.h>
#include <vtkContourFilter.h>
#include <vtkUnstructuredGridGeometryFilter.h>
#include "vtkActor.h"
#include "vtkfigTriContour2D.h"

namespace vtkfig
{
  TriContour2D::TriContour2D(): vtkfig::Figure()
  {
    RGBTable surface_rgb={{0,0,0,1},{1,1,0,0}};
    RGBTable contour_rgb={{0,0,0,0},{1,0,0,0}};
    surface_lut=BuildLookupTable(surface_rgb,255);
    contour_lut=BuildLookupTable(contour_rgb,2);
    isocontours = vtkSmartPointer<vtkContourFilter>::New();
  }
  
  void TriContour2D::RTBuild()
  {
    // filter to geometry primitive
    
    // see http://www.vtk.org/Wiki/VTK/Examples/Cxx/PolyData/GeometryFilter



    vtkSmartPointer<vtkGeometryFilter> geometry =  vtkSmartPointer<vtkGeometryFilter>::New();
    geometry->SetInputDataObject(gridfunc);


    if (show_surface)
    {
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(geometry->GetOutputPort());
      mapper->UseLookupTableScalarRangeOn();
      mapper->SetLookupTable(surface_lut);
      mapper->Update();

      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->SetMapper(mapper);
      Figure::RTAddActor(plot);
      
      if (show_surface_colorbar)
        Figure::RTAddActor2D(BuildColorBar(mapper));
    }


    if (show_contour)
    {
      isocontours = vtkSmartPointer<vtkContourFilter>::New();
      isocontours->SetInputConnection(geometry->GetOutputPort());
      
      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(isocontours->GetOutputPort());
      mapper->UseLookupTableScalarRangeOn();
      mapper->SetLookupTable(contour_lut);
      
      vtkSmartPointer<vtkActor>     plot = vtkSmartPointer<vtkActor>::New();
      plot->SetMapper(mapper);
      Figure::RTAddActor(plot);
      if (show_contour_colorbar)
        Figure::RTAddActor2D(BuildColorBar(mapper));

    }

  }

}

