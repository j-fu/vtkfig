//
// Development example for server-client communication via sockets
// using vtkfig communicator
//

#include <thread>
#include <cstring>
#include <cstdio>
#include "vtkSmartPointer.h"
#include "vtkfigCommunicator.h"



int main(int argc, const char * argv[])
{
  bool server=true;
  int port=35000;
  
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

  vtkSmartPointer<vtkfig::internals::Communicator> sccomm=vtkSmartPointer<vtkfig::internals::Communicator>::New();
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
