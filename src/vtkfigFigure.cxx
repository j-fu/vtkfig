
#include "vtkfigCommunicator.h"
#include "vtkfigFigure.h"

namespace vtkfig
{


  
  ////////////////////////////////////////////////
  Figure::Figure() {};
  void Figure::AddActor(vtkSmartPointer<vtkActor> prop) {actors.push_back(prop);}
  void Figure::AddActor2D(vtkSmartPointer<vtkActor2D> prop) {actors2d.push_back(prop);}
  bool Figure::IsEmpty(void) {return (actors.size()==0 && actors2d.size()==0);}



}
