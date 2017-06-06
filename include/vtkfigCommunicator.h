#ifndef VTKFIG_COMMUNICATOR_H
#define VTKFIG_COMMUNICATOR_H


#include "vtkSocketController.h"
#include "vtkServerSocket.h"
#include "vtkSocketCommunicator.h"


namespace vtkfig
{

  namespace internals
  {
    ///
    /// Derived class from  vtkSocketCommunicator
    /// providing framework for server-client communication
    /// 
    class Communicator: public vtkSocketCommunicator
    {
      
    public:
      enum class Command: int
      {
        Dummy=-1,
          Empty=0,
          MainThreadShow=100,
          MainThreadAddFrame,
          MainThreadRemoveFrame,
          FrameAddFigure,
          FrameSize,
          FramePosition,
          FrameSingleView,
          FrameVisibleSubFrame,
          FrameDump,
          FrameTitle,
          WindowTitle,
          FrameLinkCamera,
          FrameLayout,
          MainThreadTerminate,
          String,
          Clear,
          Exit
          };
    
      
      static Communicator *New();
      
      Communicator();
      
      ~Communicator();
      
      int ServerConnect(const int port);
      
      int ClientConnect(const char * server, const int port);
      
      int SendCommand(Command  cmd, int tag=static_cast<int>(Tag::Command));
      
      int ReceiveCommand(Command & cmd, int tag= static_cast<int>(Tag::Command));
      
      int SendInt(int  i, int tag=static_cast<int>(Tag::Int) );
      
      int ReceiveInt(int &i, int tag= static_cast<int>(Tag::Int));
      
      int SendFloat(float d, int tag= static_cast<int>(Tag::Float));
      
      int ReceiveFloat(float &d, int tag= static_cast<int>(Tag::Float));
      
      int SendDouble(double d, int tag= static_cast<int>(Tag::Double));
      
      int ReceiveDouble(double &d, int tag= static_cast<int>(Tag::Double));
      
      int SendString(const std::string  s, int tag=static_cast<int>(Tag::String) );
      
      int ReceiveString(std::string &s, int tag= static_cast<int>(Tag::String));
      
      int SendCharBuffer(char *buf, int ndata, int tag=static_cast<int>(Tag::CharBuffer) );
      
      int ReceiveCharBuffer(char *buf, int ndata, int tag= static_cast<int>(Tag::CharBuffer));
      
      int SendFloatBuffer(float *buf, int ndata, int tag=static_cast<int>(Tag::FloatBuffer) );
      
      int ReceiveFloatBuffer(float *buf, int ndata, int tag= static_cast<int>(Tag::FloatBuffer));
      
      int SendDoubleBuffer(double *buf, int ndata, int tag=static_cast<int>(Tag::DoubleBuffer) );
      
      int ReceiveDoubleBuffer(double *buf, int ndata, int tag= static_cast<int>(Tag::DoubleBuffer));

      
      
      enum class Tag : int
      {
        Command=100,
          Int,
          Float,
          Double,
          String,
          CharBuffer,
          FloatBuffer,
          DoubleBuffer
          };
      
    private:
      friend class MainThread;
      friend class Frame;
      friend class Client;

      const int remoteHandle=1;
      
      int server_listen_num_retry=10;
      int server_listen_retry_timeout=100;
      int server_listen_waiting_time=200;
      
      int client_connect_num_retry=10;
      int client_connect_retry_timeout=100;
      
      vtkSmartPointer<vtkSocketController> controller;
      vtkSmartPointer<vtkServerSocket> ssocket;
      
    };
  }
}

#endif
