#include "vtkfigServerConnection.h"
#include <stdexcept>

namespace vtkfig 
{
  
  void ServerConnection::Open(int port, int wtime)
  {
    if (is_open) return;

    cout << "Server start listening on port "<< port << endl;
    communicator=vtkSmartPointer<vtkfig::Communicator>::New();
    communicator->server_listen_num_retry=1;
    communicator->server_listen_waiting_time=1000*wtime;
    int rc=communicator->ServerConnect(port);
    if (rc)  
      cout << "Server connected" << endl;
    else
    {
      cout << "Nobody is listening ... giving up" << endl;
      throw std::runtime_error("Server connection failed");
      exit(1);
    }
    is_open=true;
  }


  ServerConnection::ServerConnection()
  {
    int wtime=5;
    char* port_string=getenv("VTKFIG_PORT_NUMBER");
    char* wtime_string=getenv("VTKFIG_WAIT_SECONDS");

    if (wtime_string!=0)
      wtime=atoi(wtime_string);
      

    if (port_string!=0)
    {
      int port=atoi(port_string);
      if (port<=0) 
        throw std::runtime_error("Invalid port number for server");
      Open(port,wtime);
    }

  }
 
  ServerConnection::~ServerConnection()
  {
    if (is_open)
      communicator->SendCommand(vtkfig::Command::Exit);  
    communicator->CloseConnection();
    is_open=false;
  }
}
