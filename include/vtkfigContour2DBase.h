#ifndef VTKFIG_CONTOUR2DBASE_H
#define VTKFIG_CONTOUR2DBASE_H
#include "vtkSliderWidget.h"
#include "vtkRenderer.h"
#include "vtkSliderRepresentation.h"
#include "vtkCommand.h"
#include "vtkContourFilter.h"
#include "vtkfigFigure.h"
#include "vtkfigTools.h"
#include "vtkPolyDataAlgorithm.h"
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

    void ShowSurface(bool b) {state.show_surface=b;}
    void ShowContour(bool b) {state.show_contour=b;}
    void ShowSlider(bool b) {state.show_slider=b;}
    void ShowSurfaceColorbar(bool b) {state.show_surface_colorbar=b;}
    void ShowContourColorbar(bool b) {state.show_contour_colorbar=b;}
    void ShowElevation(bool b) {state.show_elevation=b;}
    void SetValueRange(double vmin, double vmax){state.vmin_set=vmin; state.vmax_set=vmax;}
    void SetNumberOfIsocontours(int n) {state.ncont=n;}
    void SetXYAspect(double a) {state.aspect=a;}
    void KeepXYAspect(bool b) {state.keep_aspect=b;}

    
    vtkSmartPointer<vtkContourFilter> isocontours;

  protected:
    void RTSetInteractor(vtkSmartPointer<vtkRenderWindowInteractor> i,vtkSmartPointer<vtkRenderer> r);
    vtkSmartPointer<vtkSliderWidget> sliderWidget;
    
    vtkSmartPointer<vtkLookupTable> surface_lut;
    vtkSmartPointer<vtkLookupTable> contour_lut;

    void ProcessData(    vtkSmartPointer<vtkRenderWindowInteractor> interactor,
                         vtkSmartPointer<vtkRenderer> renderer,
                         vtkSmartPointer<vtkPolyDataAlgorithm> data, double bounds[6]);


    void SetVMinMax(double vmin, double vmax)
    {
      if (state.vmin_set<state.vmax_set)
      {
        state.real_vmin=state.vmin_set;
        state.real_vmax=state.vmax_set;
      }
      else
      {
        state.real_vmin=vmin;
        state.real_vmax=vmax;
      }

      surface_lut->SetTableRange(state.real_vmin,state.real_vmax);
      surface_lut->Modified();
      contour_lut->SetTableRange(state.real_vmin,state.real_vmax);
      contour_lut->Modified();
      
      double tempdiff = (state.real_vmax-state.real_vmin)/(1.0e4*state.ncont);
      isocontours->GenerateValues(state.ncont, state.real_vmin+tempdiff, state.real_vmax-tempdiff);
      isocontours->Modified();

    }
    void AddSlider(vtkSmartPointer<vtkRenderWindowInteractor> i,vtkSmartPointer<vtkRenderer> r);

    friend class mySliderCallback;

    struct
    {
      double vmin_set=1.0e100;
      double vmax_set=-1.0e100;
      double real_vmin=0;
      double real_vmax=1;
      
      int ncont=10;
      
      bool keep_aspect=true;
      double aspect=1.0;
      
      bool show_surface=true;
      bool show_contour=true;
      bool show_slider=false;
      bool show_surface_colorbar=true;
      bool show_contour_colorbar=false;
      bool show_elevation=true;
    } state;
    
    
    
  };






}
#endif
