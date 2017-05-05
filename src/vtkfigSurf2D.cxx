#include "vtkActor.h"
#include "vtkProperty.h"
#include "vtkTextProperty.h"
#include "vtkOutlineFilter.h"
#include "vtkStructuredGridGeometryFilter.h"
#include "vtkAxesActor.h"
#include "vtkCaptionActor2D.h"

#include "vtkfigSurf2D.h"

namespace vtkfig
{
////////////////////////////////////////////////////////////
  Surf2D::Surf2D(): Figure()
  {
    RGBTable surface_rgb={{0,0,0,1},{1,1,0,0}};
    lut=BuildLookupTable(surface_rgb,255);
    warp = vtkSmartPointer<vtkWarpScalar>::New();
    gridfunc= vtkSmartPointer<vtkStructuredGrid>::New();
  }


  void Surf2D::RTBuild()
  {

    bool do_warp=false;
    bool draw_box=true;
    bool draw_axes=true;
    bool draw_colorbar=true;
   
    // filter to geometry primitive
    vtkSmartPointer<vtkStructuredGridGeometryFilter>geometry =
      vtkSmartPointer<vtkStructuredGridGeometryFilter>::New();
    geometry->SetInputDataObject(gridfunc);

    // warp to fit in box
    if (do_warp)
    {
	warp->SetInputConnection(geometry->GetOutputPort());
    }

    // map gridfunction
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    if (do_warp)
	mapper->SetInputConnection(warp->GetOutputPort());
    else
	mapper->SetInputConnection(geometry->GetOutputPort());


    // create plot surface actor
    vtkSmartPointer<vtkActor> surfplot = vtkSmartPointer<vtkActor>::New();
    surfplot->SetMapper(mapper);
    mapper->SetLookupTable(lut);
    mapper->UseLookupTableScalarRangeOn();
    

    // create outline
    vtkSmartPointer<vtkOutlineFilter> outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
    if (do_warp)
	outlinefilter->SetInputConnection(warp->GetOutputPort());
    else
	outlinefilter->SetInputConnection(geometry->GetOutputPort());

    vtkSmartPointer<vtkPolyDataMapper>outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    outlineMapper->SetInputConnection(outlinefilter->GetOutputPort());
    vtkSmartPointer<vtkActor> outline = vtkSmartPointer<vtkActor>::New();
    outline->SetMapper(outlineMapper);
    outline->GetProperty()->SetColor(0, 0, 0);

    // create axes
    vtkSmartPointer<vtkAxesActor> axes = vtkSmartPointer<vtkAxesActor>::New();
    axes->SetShaftTypeToCylinder();
    axes->SetNormalizedShaftLength( 0.85, 0.85, 0.85);
    axes->SetNormalizedTipLength( 0.15, 0.15, 0.15);
    axes->SetCylinderRadius( 0.500 * axes->GetCylinderRadius() );
    axes->SetConeRadius( 1.025 * axes->GetConeRadius() );
    axes->SetSphereRadius( 1.500 * axes->GetSphereRadius() );
    vtkSmartPointer<vtkTextProperty> text_prop_ax = axes->GetXAxisCaptionActor2D()->
	GetCaptionTextProperty();
    text_prop_ax->SetColor(0.0, 0.0, 0.0);
    text_prop_ax->SetFontFamilyToArial();
    text_prop_ax->SetFontSize(8);
    axes->GetYAxisCaptionActor2D()->SetCaptionTextProperty(text_prop_ax);
    axes->GetZAxisCaptionActor2D()->SetCaptionTextProperty(text_prop_ax);



    // renderer
    Figure::RTAddActor(surfplot);
    if (draw_box)
	Figure::RTAddActor(outline);
//    if (draw_axes)	Figure::AddActor(axes);
    if (show_colorbar)
      Figure::RTAddActor2D(BuildColorBar(mapper));
    
  }

}
