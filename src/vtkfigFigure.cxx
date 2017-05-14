#include "vtkfigFigure.h"
namespace vtkfig
{
  ////////////////////////////////////////////////
  Figure::Figure() {};
  void Figure::RTAddActor(vtkSmartPointer<vtkActor> prop) {actors.push_back(prop);}
  void Figure::RTAddActor2D(vtkSmartPointer<vtkActor2D> prop) {actors2d.push_back(prop);}
  void Figure::RTAddContextActor(vtkSmartPointer<vtkContextActor> prop) {ctxactors.push_back(prop);}
  bool Figure::IsEmpty(void) {return (actors.size()==0 && actors2d.size()==0&& ctxactors.size()==0);}
}
