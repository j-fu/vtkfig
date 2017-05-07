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

    /// Connection is opened automatically 
    /// if VTKFIG_PORT_NUMBER is found in the
    /// environment. Otherwise it is constructed as an object, bur
    /// remains closed.
    ServerConnection();


    void Open(int port, int wtime);



    ~ServerConnection();
  };
}

#endif

