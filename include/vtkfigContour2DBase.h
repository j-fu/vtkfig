#ifndef VTKFIG_CONTOUR2DBASE_H
#define VTKFIG_CONTOUR2DBASE_H
#include "vtkSliderWidget.h"
#include "vtkRenderer.h"
#include "vtkSliderRepresentation.h"
#include "vtkCommand.h"
#include "vtkContourFilter.h"
#include "vtkfigFigure.h"
#include "vtkfigTools.h"

namespace vtkfig
{

    
 
  
  class Contour2DBase: public Figure
  {
  public:
    Contour2DBase()
    {
      RGBTable surface_rgb={{0,0,0,1},{1,1,0,0}};
      RGBTable contour_rgb={{0,0,0,0},{1,0,0,0}};
      surface_lut=BuildLookupTable(surface_rgb,255);
      contour_lut=BuildLookupTable(contour_rgb,2);
      isocontours = vtkSmartPointer<vtkContourFilter>::New();
      sliderWidget = vtkSmartPointer<vtkSliderWidget>::New();
    }
  

    void SetSurfaceRGBTable(RGBTable & tab, int tabsize)
    {
      surface_lut=BuildLookupTable(tab,tabsize);
    }
    void SetContourRGBTable(RGBTable & tab, int tabsize)
    {
      contour_lut=BuildLookupTable(tab,tabsize);
    }

    void ShowSurface(bool b) {show_surface=b;}
    void ShowContour(bool b) {show_contour=b;}
    void ShowSlider(bool b) {show_slider=b;}
    void ShowSurfaceColorbar(bool b) {show_surface_colorbar=b;}
    void ShowContourColorbar(bool b) {show_contour_colorbar=b;}
    void ShowElevation(bool b) {show_elevation=b;}
    
    
    double vmax=0;
    double vmin=0;
    int ncont=10;
    vtkSmartPointer<vtkContourFilter> isocontours;

  protected:
    void RTSetInteractor(vtkSmartPointer<vtkRenderWindowInteractor> i,vtkSmartPointer<vtkRenderer> r);
    vtkSmartPointer<vtkSliderWidget> sliderWidget;
    
    vtkSmartPointer<vtkLookupTable> surface_lut;
    vtkSmartPointer<vtkLookupTable> contour_lut;

    
    bool show_surface=true;
    bool show_contour=true;
    bool show_slider=true;
    bool show_surface_colorbar=true;
    bool show_contour_colorbar=false;
    bool show_elevation=true;
    void AddSlider(vtkSmartPointer<vtkRenderWindowInteractor> i,vtkSmartPointer<vtkRenderer> r);
    
  };






}
#endif
