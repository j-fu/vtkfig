#include <cstdio>
#include <thread>
#include "vtkfigCommunicator.h"

namespace vtkfig
{
  Communicator *Communicator::New()  {return new Communicator();}
  
  Communicator::Communicator(): vtkSocketCommunicator()
  {
    controller =vtkSmartPointer<vtkSocketController>::New();
    controller->SetCommunicator(this);
    controller->Initialize();
  }
  
  Communicator::~Communicator()
  {
    CloseConnection();
  }
  
  void Communicator::ServerConnect(const int port)
  {
    
    ssocket=vtkSmartPointer<vtkServerSocket>::New();
    
    int rc;
    rc=ssocket->CreateServer(port);
    // here, rc=0 means success...
    if (rc) throw std::runtime_error("vtkServerSocket::CreateServer failed\n");
    
    rc=0;
    int iretry=0;
    while (iretry<server_listen_num_retry && rc==0)
    {
      if (iretry) 
      {
        std::this_thread::sleep_for (std::chrono::milliseconds(server_listen_retry_timeout));
        cout << "Retry listening on port " << port << endl;
      }
      rc=WaitForConnection(ssocket,server_listen_waiting_time);
      iretry++;
    }
    if (!rc) throw std::runtime_error("Server connection failed\n");
    
    rc=Handshake();
    if (!rc) throw std::runtime_error("Server handshake failed\n");
    
  }
  
  void Communicator::ClientConnect(const char * server, const int port)
  {
    int rc=0;
    int iretry=0;
    
    while (iretry<client_connect_num_retry && rc==0)
    {
      if (iretry) 
      {
        std::this_thread::sleep_for (std::chrono::milliseconds(client_connect_retry_timeout));
        cout << "Retry connecting to " << server<<":"<< port << endl;
      }
      rc=ConnectTo(server,port);
      iretry++;
    }
    if (!rc) throw std::runtime_error("Client connection failed");
    
    
    rc=Handshake();
    if (!rc) throw std::runtime_error("Client handshake failed");
    
  }
  
}

