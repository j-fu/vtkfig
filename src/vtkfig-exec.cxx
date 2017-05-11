///
/// Client code for server-client interaction
/// 

#include <thread>
#include <cassert>
#include <memory>
#include "vtkSmartPointer.h"

#include "vtkfigFrame.h"
#include "vtkfigCommunicator.h"
#include "vtkfigSurf2D.h"
#include "vtkfigXYPlot.h"
#include "vtkfigMainThread.h"



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


namespace vtkfig
{
  
  class Client
  {

    int debug_level=0;
    int port=35000;
    std::string remotecmd;
    std::string hostname;
    int wtime=500;
    
    bool have_hostname=false;
    bool use_ssh=false;
    int iarg=1;
    bool remoteswitch=false;
    bool via=false;
    std::string via_hostname;
    vtkSmartPointer<vtkfig::Communicator> communicator;
    
  public:
    Client(int argc, const char * argv[])
    {
      
      vtkfig::MainThread::CreateMainThread();
      char *debug_string=getenv("VTKFIG_DEBUG_LEVEL");
      if (debug_string!=0)
        debug_level=atoi(debug_string);
      
      
      
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
        
        system(systemcmd.c_str());
        std::this_thread::sleep_for (std::chrono::milliseconds(wtime));
      }   
      
      communicator=vtkSmartPointer<vtkfig::Communicator>::New();
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
        throw std::runtime_error("Giving up");
      }
      
    }
    int spin()
    {
      while (1)
      {
        vtkfig::Communicator::Command cmd;
        std::shared_ptr<vtkfig::Figure> figure;
        std::shared_ptr<vtkfig::Frame> frame;
        int framenum=-1;
        
        
        communicator->ReceiveCommand(cmd);
        communicator->ReceiveInt(framenum);
        if (debug_level>0) 
          cout << "c cmd: " << static_cast<int>(cmd) << " frame: " << framenum << endl;
        
        if (framenum>=0)
          frame=vtkfig::MainThread::mainthread->framemap[framenum];
        
        switch(cmd)
        {
          
        case vtkfig::Communicator::Command::Dummy:
        {
          if (debug_level>0)
            cout << "Dummy" << endl;
        }
        break;
        
        case vtkfig::Communicator::Command::Clear:
        {
          if (debug_level>0)
            cout << "clear" << endl;
        }
        break;
        
        case vtkfig::Communicator::Command::String:
        {
          std::string s;
          communicator->ReceiveString(s);
          
          if (debug_level>0)
            cout << "String: " <<  s << endl;
        }
        break;
        
        case vtkfig::Communicator::Command::Exit:
        {
          cout << "Exit by request" << endl;
          return 0;
        }
        break;
        
        case vtkfig::Communicator::Command::MainThreadAddFrame:
        {
          int nrow, ncol;
//      assert(framenum==-1);
          communicator->ReceiveInt(nrow);
          communicator->ReceiveInt(ncol);
          auto frame=vtkfig::Frame::New(nrow,ncol);
          
          if (debug_level>0)
            cout << "New frame" << endl;
        }
        break;
        
        case vtkfig::Communicator::Command::FrameAddFigure:
        {
          std::string figtype;
          communicator->ReceiveString(figtype);
          int irow, icol;
          communicator->ReceiveInt(irow);
          communicator->ReceiveInt(icol);
          
          if (figtype=="Surf2D")
          {
            figure=vtkfig::Surf2D::New();
            frame->AddFigure(figure,irow,icol);
            if (debug_level>0)
              cout << "Add Surf2d" << endl;
          }
          else if (figtype=="XYPlot")
          {
            figure=vtkfig::XYPlot::New();
            frame->AddFigure(figure,irow,icol);
            if (debug_level>0)
              cout << "Add XYPlot" << endl;
          }
          else
          {
            cout << "Communication error addfig: type"<< figtype << endl;
            return 0;
          }
        }
        break;
        
        case vtkfig::Communicator::Command::MainThreadShow:
        {
          for (auto & framepair: vtkfig::MainThread::mainthread->framemap)
          {
            for (auto figure: framepair.second->figures)
            {
              figure->ClientMTReceive(communicator);
            }
          }
          vtkfig::MainThread::mainthread->Show();
        }
        break;
        
        case vtkfig::Communicator::Command::FrameSize:
        {
          int x,y;
          communicator->ReceiveInt(x);
          communicator->ReceiveInt(y);
          frame->Size(x,y);
        }
        break;
        
        case vtkfig::Communicator::Command::FramePosition:
        {
          int x,y;
          communicator->ReceiveInt(x);
          communicator->ReceiveInt(y);
          frame->Position(x,y);
        }
        break;
        
        case vtkfig::Communicator::Command::FrameDump:
        {
          std::string fname;
          communicator->ReceiveString(fname);
          frame->Dump(fname);
        }
        break;
        
        case vtkfig::Communicator::Command::MainThreadTerminate:
        {
          if (debug_level>0)
            cout << "c term" << endl;
          vtkfig::MainThread::mainthread->Terminate();
          return 0;
        }
        break;

        
        default:
        {
          throw std::runtime_error("wrong command on client");
          return 0;
        }
        
        }
        
      }
      
    }
  };
  
}  



int main(int argc, const char * argv[])
{
  vtkfig::Client client(argc,argv);
  return client.spin();
}
