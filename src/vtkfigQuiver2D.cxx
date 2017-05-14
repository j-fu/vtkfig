#include "vtkActor.h"
#include "vtkPointData.h"
#include "vtkProperty.h"
#include "vtkTextProperty.h"
#include "vtkFloatArray.h"
#include "vtkContourFilter.h"
#include "vtkOutlineFilter.h"
#include "vtkRectilinearGridGeometryFilter.h"
#include "vtkGlyph3D.h"
#include "vtkRectilinearGrid.h"
#include "vtkGlyphSource2D.h"


#include "vtkfigQuiver2D.h"

namespace vtkfig
{
////////////////////////////////////////////////////////////


  ////////////////////////////////////////////////////////////
  Quiver2D::Quiver2D(): Figure()  
  {  
    RGBTable quiver_rgb={{0,0,0,1},{1,1,0,0}};
    lut=BuildLookupTable(quiver_rgb,255);
  }
  
  void  Quiver2D::RTBuild(
        vtkSmartPointer<vtkRenderWindow> window,
        vtkSmartPointer<vtkRenderWindowInteractor> interactor,
        vtkSmartPointer<vtkRenderer> renderer)
  {
    vtkSmartPointer<vtkRectilinearGrid> gridfunc= vtkSmartPointer<vtkRectilinearGrid>::New();
    int Nx = xcoord->GetNumberOfTuples();
    int Ny = ycoord->GetNumberOfTuples();
    gridfunc->SetDimensions(Nx, Ny, 1);
    gridfunc->SetXCoordinates(xcoord);
    gridfunc->SetYCoordinates(ycoord);
    gridfunc->GetPointData()->SetScalars(colors);
    gridfunc->GetPointData()->SetVectors(vectors);

    // filter to geometry primitive
    vtkSmartPointer<vtkRectilinearGridGeometryFilter> geometry =
      vtkSmartPointer<vtkRectilinearGridGeometryFilter>::New();
    geometry->SetInputDataObject(gridfunc);

    // make a vector glyph
    vtkSmartPointer<vtkGlyphSource2D> vec = vtkSmartPointer<vtkGlyphSource2D>::New();
    vec->SetGlyphTypeToArrow();

    vec->SetScale(arrow_scale);
    vec->FilledOff();

    vtkSmartPointer<vtkGlyph3D> glyph = vtkSmartPointer<vtkGlyph3D>::New();
    glyph->SetInputConnection(geometry->GetOutputPort());
    glyph->SetSourceConnection(vec->GetOutputPort());
    glyph->SetColorModeToColorByScalar();

    // map gridfunction
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(glyph->GetOutputPort());
    mapper->SetLookupTable(lut);
    mapper->UseLookupTableScalarRangeOn();


    // create plot quiver actor
    vtkSmartPointer<vtkActor> quiver_actor = vtkSmartPointer<vtkActor>::New();
    quiver_actor->SetMapper(mapper);



    // create outline
    // vtkSmartPointer<vtkOutlineFilter>outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
    // outlinefilter->SetInputConnection(geometry->GetOutputPort());
    // vtkSmartPointer<vtkPolyDataMapper> outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    // outlineMapper->SetInputConnection(outlinefilter->GetOutputPort());
    // vtkSmartPointer<vtkActor> outline = vtkSmartPointer<vtkActor>::New();
    // outline->SetMapper(outlineMapper);
    // outline->GetProperty()->SetColor(0, 0, 0);
    // Figure::AddActor(outline);

    // add actors to renderer
    Figure::RTAddActor(quiver_actor);
    if (show_colorbar)
      Figure::RTAddActor2D(BuildColorBar(mapper));
  }

}
