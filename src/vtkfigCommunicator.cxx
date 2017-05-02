#include "vtkfigCommunicator.h"

namespace vtkfig
{
  Communicator::Communicator():vtkObjectBase() 
  {
    figures=std::make_shared<std::vector<std::shared_ptr<Figure>>>();
  };
  
  
  /// VTK style static constructor
  Communicator *Communicator::New()   {      return new Communicator;    }
}

