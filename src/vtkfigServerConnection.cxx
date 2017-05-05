#include "vtkfigServerConnection.h"
#include <stdexcept>

namespace vtkfig 
{
  
  ServerConnection::ServerConnection(const int argc, const char *argv[])
  {
    int port=35000;
    int wtime=10;
    int iarg=1;
    bool do_connect=false;
    while (iarg<argc)
    {
      if (strcmp(argv[iarg],"-s")==0) 
      {
        do_connect=true;
        iarg++;
        continue;
      }
      
      if (strcmp(argv[iarg],"-p")==0 && (iarg+1<argc)) 
      {
        port=atoi(argv[iarg+1]); 
        iarg+=2; 
        continue;
      }
      
      if (strcmp(argv[iarg],"-t")==0 && (iarg+1<argc)) 
      {
        wtime=atoi(argv[iarg+1]); 
        iarg+=2; 
        continue;
      }
      
      cout << "I don't understand. What do you mean by " << argv[iarg] << " ?" << endl;
      throw std::runtime_error("Unknown argument");
      
    }
    if (!do_connect) return;
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
    }
    is_open=true;
  }
 
  ServerConnection::~ServerConnection()
  {
    if (is_open)
      communicator->SendCommand(vtkfig::Command::Exit);  
    communicator->CloseConnection();
    is_open=false;
  }
}
