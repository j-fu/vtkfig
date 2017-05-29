//
// Learning example for server-client communication via sockets
// using vtk infrastructure
//
#include <cstdio>
#include <thread>
#include <cassert>
#include <cstdio>

#include "vtkSocketController.h"
#include "vtkServerSocket.h"
#include "vtkSocketCommunicator.h"
#include "vtkSmartPointer.h"
#include "vtkPoints.h"

int main(int argc, const char * argv[])
{
  int rc;
  const int port=35000;
  
  char xname[10];
  bool server=true;

  strncpy(xname,"server",10);

  // if called with -c run as client.
  if (argc>1 && argv[1][0]=='-'&& argv[1][1]=='c')
  {
    strcpy(xname,"client");
    server=false;
  }

  printf("%s start\n",xname);
  

  // Use server socket when establishing connection
  // this allows for a timeout in wait. Otherwis
  // wait would be blocking.
  bool use_ssocket=true;
  vtkSmartPointer<vtkServerSocket> ssocket;
  if (server)
  {

    /// Start client
    char command[256];
    snprintf(command,256,"%s -c &",argv[0]);
    std::system(command);
  }

  
  vtkSmartPointer<vtkSocketController> controller =vtkSmartPointer<vtkSocketController>::New();
  vtkSmartPointer<vtkSocketCommunicator> comm =vtkSmartPointer<vtkSocketCommunicator>::New();

  controller->SetCommunicator(comm);
  controller->Initialize();
//  comm->SetReportErrors(0);
  
  
  int nwait=10;
  int iwait=0;
  if (server)
  {

//    comm->LogToFile("server.log");
    if (use_ssocket)
    {
      ssocket=vtkSmartPointer<vtkServerSocket>::New();
      rc=ssocket->CreateServer(port);
      printf("%s vtkServerSocket::CreateServer: rc=%d\n",xname, rc);
     assert(rc==0);
    }
    
    rc=0;
    while (iwait<nwait && rc==0)
    {
      printf("%s wait...\n",xname);
      if (use_ssocket)
        rc=comm->WaitForConnection(ssocket,100);
      else
        rc=comm->WaitForConnection(port);

      printf("%s wait: rc=%d\n",xname, rc);
      std::this_thread::sleep_for (std::chrono::milliseconds(100));
      iwait++;
    }
  }
  else
  {
//    comm->LogToFile("client.log");
    rc=0;
    while (iwait<nwait && rc==0)
    {
      printf("%s connect to ...\n",xname);
      rc=comm->ConnectTo("localhost",port);
      printf("%s connect: rc=%d\n",xname, rc);
      iwait++;
    }
  }

  if (rc!=0)
  {
    printf("%s connected \n",xname);
  }
  else
  {
    printf("%s give up waiting\n",xname);
    return 0;
  }

  rc=comm->Handshake();
  printf("%s handshake %d\n",xname,rc);

  if (server)
    assert(comm->GetIsServer());
  else
    assert(!comm->GetIsServer());


  int iproc=comm->GetLocalProcessId();
  int nproc=comm->GetNumberOfProcesses();
  printf("%s: nproc=%d, iproc=%d\n",xname, nproc,iproc);

  // There seems no way to set the correct Ids
  // see http://www.vtk.org/doc/nightly/html/classvtkSocketController.html
  // search for "FOOLISH MORTALS!"

  // Consequently, if I am 0 "the other" always seems to be 1
  // (this was guessed...)
  int remoteHandle=1;

  
  comm->Barrier();
  printf("%s Barrier\n",xname);

  
  int zahl=0;
  // Send data server->client
  int tag=123;
  if (server)
  {
    zahl=42;
    rc=comm->Send(&zahl,1,remoteHandle,tag);
    printf("%s send %d: %d\n",xname,zahl, rc);
    std::this_thread::sleep_for (std::chrono::milliseconds(100));
  }
  else
  {
    zahl=0;
    rc=comm->Receive(&zahl,1,remoteHandle,tag);
    printf("%s receive %d: %d\n",xname,zahl, rc);
  }

  // Send data client->server
  tag=124;
  if (!server)
  {
    zahl=42;
    rc=comm->Send(&zahl,1,remoteHandle,tag);
    printf("%s send %d: %d\n",xname,zahl, rc);
  }
  else
  {
    zahl=0;
    rc=comm->Receive(&zahl,1,remoteHandle,tag);
    printf("%s receive %d: %d\n",xname,zahl, rc);
  }

  
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  vtkSmartPointer<vtkPoints>  points = vtkSmartPointer<vtkPoints>::New();
  
  tag=123;
  if (server)
  {
    int Nx=10;
    int Ny=10;
    
    for (int j = 0; j < Ny; j++)
      for (int i = 0; i < Nx; i++)
      {
        points->InsertNextPoint(i, j, 100*i+j);
      }
    rc=comm->Send(points->GetData(),remoteHandle,tag);
    printf("%s send %d points: %d\n",xname,static_cast<int>(points->GetNumberOfPoints()), rc);
  }
  else
  {
    rc=comm->Receive(points->GetData(),remoteHandle,tag);
    printf("%s received %d points: %d\n",xname,static_cast<int>(points->GetNumberOfPoints()), rc);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  
  for (int i=0;i<points->GetNumberOfPoints();i++)
  {
    double x[3];
    points->GetPoint(i,x);
    printf("%s: point %03.1f %03.1f %03.1f\n",xname,x[0],x[1],x[2]);
  }
  printf("\n");

  comm->Barrier();


  rc=comm->GetIsConnected();
  printf("%s connected %d\n",xname,rc);
  rc=comm->Handshake();
  printf("%s handshake %d\n",xname,rc);


  comm->Barrier();
  printf("%s Barrier\n",xname);


  
  // What does the server feel if the client exits ?
  if (server)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    rc=comm->GetIsConnected();
    printf("%s connected %d\n",xname,rc);
    rc=comm->Handshake();
    printf("%s handshake %d\n",xname,rc);
    if (!rc)
      printf("%s lost connection\n",xname);
      
  }
  comm->CloseConnection();
  printf("%s stop\n",xname);
}

