#include "vtkfigFigure.h"
namespace vtkfig
{
  ////////////////////////////////////////////////
  Figure::Figure() 
  {
    surface_lut=BuildLookupTable(surface_rgbtab,state.surface_rgbtab_size);
    contour_lut=BuildLookupTable(contour_rgbtab,state.contour_rgbtab_size);
    isocontours = vtkSmartPointer<vtkContourFilter>::New();
  };

  void Figure::RTAddActor(vtkSmartPointer<vtkActor> prop) {actors.push_back(prop);}
  void Figure::RTAddActor2D(vtkSmartPointer<vtkActor2D> prop) {actors2d.push_back(prop);}
  void Figure::RTAddContextActor(vtkSmartPointer<vtkContextActor> prop) {ctxactors.push_back(prop);}
  bool Figure::IsEmpty(void) {return (actors.size()==0 && actors2d.size()==0&& ctxactors.size()==0);}
}
