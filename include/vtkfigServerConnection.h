#ifndef VTKFIG_SERVER_CONNECTION_H
#define VTKFIG_SERVER_CONNECTION_H

#include "vtkfigCommunicator.h"

namespace vtkfig
{
  class ServerConnection
  {
    bool is_open=false;
    vtkSmartPointer<Communicator> communicator;
  public:
    bool IsOpen() { return is_open;}

    vtkSmartPointer<Communicator> GetCommunicator() {return communicator;}

    ServerConnection(const int argc, const char *argv[]);

    ~ServerConnection();
  };
}

#endif

