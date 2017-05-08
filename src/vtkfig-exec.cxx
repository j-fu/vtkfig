//
// Development examples for server/client
// 

#include <thread>
#include <cassert>
#include <memory>
#include "vtkSmartPointer.h"

#include "vtkfigFrame.h"
#include "vtkfigCommunicator.h"
#include "vtkfigSurf2D.h"
#include "vtkfigXYPlot.h"



/**
Idea: 
Figure gets 2 new methods: ClientMTReceive and ServerRTSend

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

- Problem: client code won't know derived types...
 */




int main(int argc, const char * argv[])
{
  int port=35000;
  std::string remotecmd;
  std::string hostname;
  int wtime=500;
  
  bool debug=false;
  bool have_hostname=false;
  bool use_ssh=false;
  int iarg=1;
  bool remoteswitch=false;
  bool via=false;
  std::string via_hostname;
  while (iarg<argc)
  {
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

    if (strcmp(argv[iarg],"-via")==0 && (iarg+1<argc)) 
    {
      via=true;
      via_hostname.append(argv[iarg+1]);
      iarg+=2; 
      continue;
    }

    if (strcmp(argv[iarg],"-ssh")==0 && (iarg+1<argc)) 
    {
      use_ssh=true;
      iarg+=1; 
      continue;
    }

    if (strcmp(argv[iarg],"-d")==0) 
    {
      debug=true;
      iarg++;
      continue;
    }
    
    /// first "non-switch" is hostname
    {
      if (!have_hostname)
      {
        hostname.append(argv[iarg]);
        iarg++;
        have_hostname=true;
        continue;
      }
    }

    
    /// all the unrecognized rest ist the command on hostname.
    {
      remoteswitch=true;
      remotecmd.append(argv[iarg]);
      remotecmd.append(" ");
      iarg++;
    }
  }


  if (remoteswitch)
  {
    std::string systemcmd;
    if (use_ssh)
    {
      cout << "Connecting via ssh" << endl;
      systemcmd.append("ssh -q ");
      
      if (via)
      {
        cout << "Connecting via " << via_hostname <<  endl;

        systemcmd.append("-L ");
        systemcmd.append(std::to_string(port));
        systemcmd.append(":");
        systemcmd.append(hostname);
        systemcmd.append(":");
        systemcmd.append(std::to_string(port));
        systemcmd.append(" ");
        systemcmd.append(via_hostname);
        systemcmd.append(" \"ssh ");
      }
      systemcmd.append(hostname);
      if (via)
        systemcmd.append(" ");
      else
        systemcmd.append("  \" ");
    }
    
    systemcmd.append("VTKFIG_PORT_NUMBER=");
    systemcmd.append(std::to_string(port));
    systemcmd.append(" ");
    systemcmd.append(remotecmd);
    systemcmd.append(" ");
    if (use_ssh)
      systemcmd.append("\" ");
    systemcmd.append("&");

    if (via)
    {
      hostname.clear();
      hostname.append("localhost");
    }
    cout << systemcmd << endl;

//    exit(1);
    system(systemcmd.c_str());
    std::this_thread::sleep_for (std::chrono::milliseconds(wtime));
  }   
  
  vtkSmartPointer<vtkfig::Communicator> communicator=vtkSmartPointer<vtkfig::Communicator>::New();
  communicator->SetReportErrors(0);
  communicator->client_connect_num_retry=2;

  int twait=10;
  int tfac=2;
  int iretry=0;
  int nretry=10;
  int rc=0;

  while (iretry<nretry)
  {
    cout << "Client connecting to "<< hostname << ":" << port << "..." << endl;
    rc=communicator->ClientConnect(hostname.c_str(),port);
    if (rc) break;
    std::this_thread::sleep_for (std::chrono::milliseconds(twait));
    iretry++;
    twait*=tfac;
  }

  if (rc)
    cout << "Client connected to "<< hostname << ":" << port << endl;
  else
  {
    cout << "Client failed to connect to "<< hostname << ":" << port << endl;
    return 1;
  }

  std::shared_ptr<vtkfig::Frame> frame;
  std::shared_ptr<vtkfig::Figure> figure;
  
  vtkfig::Command cmd;
  while (1)
  {
    communicator->ReceiveCommand(cmd);

    switch(cmd)
    {

    case vtkfig::Command::Dummy:
    {
      if (debug)
        cout << "Dummy" << endl;
    }
    break;

    case vtkfig::Command::String:
    {
      std::string s;
      communicator->ReceiveString(s);
      
      if (debug)
        cout << "String: " <<  s << endl;
    }
    break;

    case vtkfig::Command::Exit:
    {
      cout << "Exit by request" << endl;
      return 0;
    }
    break;

    case vtkfig::Command::NewFrame:
    {
      frame=vtkfig::Frame::New();
      if (debug)
        cout << "New frame" << endl;
    }
    break;

    case vtkfig::Command::AddFigure:
    {
      std::string figtype;
      communicator->ReceiveString(figtype);
      if (figtype=="Surf2D")
      {
        figure=vtkfig::Surf2D::New();
        frame->AddFigure(figure);
        if (debug)
          cout << "Add Surf2d" << endl;
      }
      else if (figtype=="XYPlot")
      {
        figure=vtkfig::XYPlot::New();
        frame->AddFigure(figure);
        if (debug)
          cout << "Add XYPlot" << endl;
      }
      else
      {
        cout << "Communication error addfig: type"<< figtype << endl;
        return 0;
      }
    }
    break;

    case vtkfig::Command::SetInteractorStyle:
    {
      int istyle;
      communicator->ReceiveInt(istyle);
      frame->SetInteractorStyle(static_cast<vtkfig::Frame::InteractorStyle>(istyle));
    }
    break;

    case vtkfig::Command::FrameShow:
    {
      figure->ClientMTReceive(communicator);
      frame->Show();
    }

    break;

    default:
    {
      cout << "Communication error cmd: " << static_cast<int>(cmd) << endl;
      return 0;
    }

    }
    
  }
  return 0;
}
