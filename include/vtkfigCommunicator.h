#ifndef VTKFIG_COMMUNICATOR_H
#define VTKFIG_COMMUNICATOR_H


#include "vtkSocketController.h"
#include "vtkServerSocket.h"
#include "vtkSocketCommunicator.h"


namespace vtkfig
{


  enum class Command: int
  {
    Dummy=100,
      String,
      NewFrame,
      AddFigure,
      SetInteractorStyle,
      FrameShow,
      Exit
      };

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

    int ServerConnect(const int port);

    int ClientConnect(const char * server, const int port);

    int SendCommand(Command  cmd, int tag=static_cast<int>(Tag::Command));

    int ReceiveCommand(Command & cmd, int tag= static_cast<int>(Tag::Command));

    int SendInt(int  i, int tag=static_cast<int>(Tag::Int) );

    int ReceiveInt(int &i, int tag= static_cast<int>(Tag::Int));

    int SendDouble(double d, int tag= static_cast<int>(Tag::Double));

    int ReceiveDouble(double &d, int tag= static_cast<int>(Tag::Double));

    int SendString(const std::string  s, int tag=static_cast<int>(Tag::String) );

    int ReceiveString(std::string &s, int tag= static_cast<int>(Tag::String));

    int SendBuffer(char *buf, int ndata, int tag=static_cast<int>(Tag::Buffer) );

    int ReceiveBuffer(char *buf, int ndata, int tag= static_cast<int>(Tag::Buffer));


  private:
    const int remoteHandle=1;
    enum class Tag : int
    {
      Command=100,
        Int,
        Double,
        String,
        Buffer
        };
    

    vtkSmartPointer<vtkSocketController> controller;
    vtkSmartPointer<vtkServerSocket> ssocket;

  };

}

#endif
