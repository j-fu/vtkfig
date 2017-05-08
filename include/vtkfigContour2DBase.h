
#ifndef VTKFIG_CONTOUR2DBASE_H
#define VTKFIG_CONTOUR2DBASE_H
#include "vtkSliderWidget.h"
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
    void ShowSurfaceColorbar(bool b) {show_surface_colorbar=b;}
    void ShowContourColorbar(bool b) {show_contour_colorbar=b;}
    
    
    
    double vmax=0;
    double vmin=0;
    int ncont=10;
    vtkSmartPointer<vtkContourFilter> isocontours;

  protected:
    void RTSetInteractor(vtkSmartPointer<vtkRenderWindowInteractor>);
    vtkSmartPointer<vtkSliderWidget> sliderWidget;
    
    vtkSmartPointer<vtkLookupTable> surface_lut;
    vtkSmartPointer<vtkLookupTable> contour_lut;

    
    bool show_surface=true;
    bool show_contour=true;
    bool show_slider=true;
    bool show_surface_colorbar=true;
    bool show_contour_colorbar=false;
    void AddSlider();
    
  };

  class mySliderCallback : public vtkCommand
  {
  public:
    static mySliderCallback *New() 
    {
      return new mySliderCallback;
    }
    virtual void Execute(vtkObject *caller, unsigned long, void*)
    {
      vtkSliderWidget *sliderWidget = 
        reinterpret_cast<vtkSliderWidget*>(caller);
      this->contour2d->ncont=static_cast<vtkSliderRepresentation *>(sliderWidget->GetRepresentation())->GetValue();
      double tempdiff = (this->contour2d->vmax-this->contour2d->vmin)/(10*this->contour2d->ncont);
      this->contour2d->isocontours->GenerateValues(this->contour2d->ncont, this->contour2d->vmin+tempdiff, this->contour2d->vmax-tempdiff);
      this->contour2d->isocontours->Modified();
    }
  mySliderCallback():contour2d(0) {}
    Contour2DBase *contour2d;
  };






}
#endif
