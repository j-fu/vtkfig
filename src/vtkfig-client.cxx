//
// Development examples for server/client
// 

#include <thread>
#include <cassert>
#include <memory>
#include "vtkSmartPointer.h"

#include "vtkfigFrame.h"
#include "vtkfigCommunicator.h"



/**
Idea: 
Figure gets 2 new methods: ClientMTReceive and ServerRTReceive

Istead of RTBuild, render thread calls RTSend.

MTRecieve then is called on the client side in the main thread.



vtkfig::InitServer()  starts server mode.
Initially, restrict server mode to one frame.

Alternatively, there is one client which can have multiple frames. 
This is better to handle. MTSend then needs to communicate the frame number

How do we communicate the client start ?
vtkfig-hostfile

host 1 port 1
host 2 port 2


Aim: 
- server code should run robustly, whether the client is alive or not
- client should be able to connect/disconnect at any time
   -> So not only figures but also frames should be proxy objets on the server side

 */




int main(int argc, const char * argv[])
{
  bool server=true;
  int port=9805;
  
  int iarg=1;
  while (iarg<argc)
  {
    if (strcmp(argv[iarg],"-c")==0) 
    {
      server=false; 
      iarg++; 
      continue;
    }

    if (strcmp(argv[iarg],"-p")==0 && (iarg+1<argc)) 
    {
      port=atoi(argv[iarg+1]); 
      iarg+=2; 
      continue;
    }
  }

  vtkSmartPointer<vtkfig::Communicator> sccomm=vtkSmartPointer<vtkfig::Communicator>::New();
  if (server)
  {

    char command[256];
    snprintf(command,256,"%s -c -p %d&",argv[0],port);
    std::system(command);
    
    sccomm->ServerConnect(port);
    cout << "Server connect ok\n";
  }
  else
  {
    sccomm->ClientConnect("localhost",port);
    cout << "Client connect ok\n";
  }

}
