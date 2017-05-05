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
  
  int Communicator::ServerConnect(const int port)
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
      }
      rc=WaitForConnection(ssocket,server_listen_waiting_time);
      iretry++;
    }
    if (!rc) return rc;
    
    rc=Handshake();
    if (!rc) throw std::runtime_error("Server handshake failed\n");

    return rc;
  }
  
  int Communicator::ClientConnect(const char * server, const int port)
  {
    int rc=0;
    int iretry=0;
    
    while (iretry<client_connect_num_retry && rc==0)
    {
      if (iretry) 
      {
        std::this_thread::sleep_for (std::chrono::milliseconds(client_connect_retry_timeout));
      }
      rc=ConnectTo(server,port);
      iretry++;
    }
    if (!rc) return rc;
    
    
    rc=Handshake();
    if (!rc) throw std::runtime_error("Client handshake failed");
    return rc;
  }


  int Communicator::SendCommand(Command  cmd, int tag)
  {
    int icmd=static_cast<int>(cmd);
    return Send(&icmd,1,remoteHandle,tag);
  }
  
  int Communicator::ReceiveCommand(Command & cmd, int tag)
  {
    int icmd=0;
    int rc=Receive(&icmd,1,remoteHandle,tag);
    cmd=static_cast<Command>(icmd);
    return rc;
  }
  
  int Communicator::SendInt(int  i, int tag)
  {
    return Send(&i,1,remoteHandle,tag);
  }
  
  int Communicator::ReceiveInt(int &i, int tag)
  {
    return Receive(&i,1,remoteHandle,tag);
  }
  
  int Communicator::SendDouble(double d, int tag)
  {
    return Send(&d,1,remoteHandle,tag);
  }
  
  int Communicator::ReceiveDouble(double &d, int tag)
  {
    return Receive(&d,1,remoteHandle,tag);
  }
  
  int Communicator::SendString(const std::string  s, int tag)
  {
    SendInt(s.size());
    return Send(s.c_str(),s.size(),remoteHandle,tag);
  }
  
  int Communicator::ReceiveString(std::string &s, int tag)
  {
    const int bufsize=256;
    char buf[bufsize];
    int len=0;
    ReceiveInt(len);
    int rc=Receive(buf,bufsize,remoteHandle,tag);
    s.assign(buf,len);
    return rc;
  }
  
  
}
  
  
