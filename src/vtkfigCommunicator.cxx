#include "vtkfigCommunicator.h"

namespace vtkfig
{
  Communicator::Communicator():vtkObjectBase() 
  {
    actors=std::make_shared<std::vector<vtkSmartPointer<vtkProp>>>();
  };
  
  
  /// VTK style static constructor
  Communicator *Communicator::New()   {      return new Communicator;    }
}

