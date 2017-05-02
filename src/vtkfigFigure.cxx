
#include "vtkfigCommunicator.h"
#include "vtkfigFigure.h"

namespace vtkfig
{


  
  ////////////////////////////////////////////////
  Figure::Figure(): actors(std::make_shared<std::vector<vtkSmartPointer<vtkProp>>>()) {};
  void Figure::AddActor(vtkSmartPointer<vtkProp> prop) {actors->push_back(prop);}
  bool Figure::IsEmpty(void) {return (actors->size()==0);}



}
