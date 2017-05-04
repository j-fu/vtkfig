//
// Development examples for server/client
// 

#include "vtkfigFrame.h"
#include "vtkfigFigure.h"


#include <cstdio>
#include <thread>
#include <cassert>
#include <memory>

#include "vtkSocketController.h"
#include "vtkServerSocket.h"
#include "vtkSocketCommunicator.h"
#include "vtkSmartPointer.h"

/**
Idea: 
Figure gets 2 new methods: ClientMTReceive and ServerRTReceive

Istead of RTBuild, render thread calls RTSend.

MTRecieve then is called on the client side in the main thread.

 */


class SCCommunicator
{
public:
  vtkSmartPointer<vtkSocketController> controller;
  vtkSmartPointer<vtkSocketCommunicator> communicator;
  vtkSmartPointer<vtkServerSocket> ssocket;

  bool is_server=false;
  int nretry=10;
  bool is_up=false;
  
  static std::shared_ptr<SCCommunicator> New()  { return std::make_shared<SCCommunicator>();}
  SCCommunicator()
  {
    controller =vtkSmartPointer<vtkSocketController>::New();
    communicator =vtkSmartPointer<vtkSocketCommunicator>::New();
    controller->SetCommunicator(communicator);
    controller->Initialize();
    //    communicator->SetReportErrors(1);
  }
  ~SCCommunicator()
  {
    Close();
  }
  
  void InitServer(const char *client, const int port)
  {
    is_server=true;
    /// Start client
    char command[256];
    snprintf(command,256,"%s -c &",client);
    std::system(command);

    ssocket=vtkSmartPointer<vtkServerSocket>::New();

    int rc;
    rc=ssocket->CreateServer(port);
    // here, rc=0 means success...
    if (rc) throw std::runtime_error("vtkServerSocket::CreateServer failed\n");
    
    rc=0;
    int iretry=0;
    while (iretry<nretry && rc==0)
    {
      if (iretry) std::this_thread::sleep_for (std::chrono::milliseconds(100));
      rc=communicator->WaitForConnection(ssocket,100);
      iretry++;
    }
    if (!rc) throw std::runtime_error("Server connection failed");
    
    rc=communicator->Handshake();
    if (!rc) throw std::runtime_error("Server handshake failed");
    is_up=true;
  }

  void InitClient(const char * server, const int port)
  {
    int rc=0;
    int iretry=0;

    while (iretry<nretry && rc==0)
    {
      if (iretry) std::this_thread::sleep_for (std::chrono::milliseconds(100));
      rc=communicator->ConnectTo("localhost",port);
      iretry++;
    }
    if (!rc) throw std::runtime_error("Client connection failed");

    
    rc=communicator->Handshake();
    if (!rc) throw std::runtime_error("Client handshake failed");
    
    is_up=true;
  }
  void Close()
  {
    if (is_up)
      communicator->CloseConnection();
  }
};


int main(int argc, const char * argv[])
{
  bool server=true;
  int port=9805;

  if (argc>1 && argv[1][0]=='-'&& argv[1][1]=='c')
  {
    server=false;
  }

  auto sccomm=SCCommunicator::New();
  if (server)
    sccomm->InitServer(argv[0],port);
  else
    sccomm->InitClient("localhost",port);
  

  sccomm->Close();
  
}
