#ifndef VTKFIG_COMMUNICATOR_H
#define VTKFIG_COMMUNICATOR_H


#include "vtkSocketController.h"
#include "vtkServerSocket.h"
#include "vtkSocketCommunicator.h"


namespace vtkfig
{
  class Communicator: public vtkSocketCommunicator
  {
  public:
    
    int server_listen_num_retry=10;
    int server_listen_retry_timeout=100;
    int server_listen_waiting_time=200;
    
    int client_connect_num_retry=10;
    int client_connect_retry_timeout=100;
    
    static Communicator *New();
    
    Communicator();
        
    ~Communicator();

    void ServerConnect(const int port);
    void ClientConnect(const char * server, const int port);
    
  private:
    vtkSmartPointer<vtkSocketController> controller;
    vtkSmartPointer<vtkServerSocket> ssocket;

  };

}

#endif
