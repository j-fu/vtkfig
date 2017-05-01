
#include "vtkActor.h"
#include "vtkProperty.h"

#include "vtkTextProperty.h"
#include "vtkFloatArray.h"
#include "vtkContourFilter.h"
#include "vtkOutlineFilter.h"
#include "vtkRectilinearGridGeometryFilter.h"




// Quiver
#include "vtkGlyph3D.h"
#include "vtkGlyphSource2D.h"

// Contour3d
#include "vtkPlane.h"
#include "vtkCutter.h"


#include "vtkPolyDataNormals.h"


/*
#include "vtkCaptionActor2D.h"
#include "vtkDataSetMapper.h"
#include "vtkGlyphSource2D.h"
#include "vtkPolyData.h"
#include "vtkTubeFilter.h"
*/

#include "vtkfig.h"

namespace vtkfig
{
////////////////////////////////////////////////////////////


  ////////////////////////////////////////////////////////////
  Quiver2D::Quiver2D(): Figure()  
  {  
    RGBTable quiver_rgb={{0,0,0,1},{1,1,0,0}};
    lut=BuildLookupTable(quiver_rgb,255);
  }
  
  void  Quiver2D::Add(const vtkSmartPointer<vtkFloatArray> xcoord,
                      const vtkSmartPointer<vtkFloatArray> ycoord,
                      const vtkSmartPointer<vtkFloatArray> colors,
                      const vtkSmartPointer<vtkFloatArray> vectors)
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

    double vrange[2];
    gridfunc->GetScalarRange(vrange);
    mapper->SetScalarRange(vrange[0], vrange[1]);

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
    Figure::AddActor(quiver_actor);
    if (show_colorbar)
      Figure::AddActor(Figure::BuildColorBar(mapper));
  }
  


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



