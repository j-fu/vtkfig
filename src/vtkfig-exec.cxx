/// \file vtkfig-exec.cxx
///
/// Client code for server-client interaction
/// 

#include <string>
#include "config.h"

static const std::string  helpstring=R"(
vtkfig-exec [-p portnumber] [-t timeout] [-via host] [-ssh] hostname command

Run command on host and send all vtkfig rendering commands to local host.

Parameters:
   -p   portnumber  Port number on remote host for connection (default: 35000)
   -t   timeout     Timeout for establishing connection in ms (default: 1000)
   -via vianame     Name of via host (for ssh tunneling)
   -ssh             Connet via ssh 
        hostname    Name of host to run command on (mandatory)
        command     command to run (mandatory; all the remaining part of the command line): 
   -h               This help.
)";

#include <thread>
#include <cassert>
#include <memory>
#include <vtkSmartPointer.h>

#include "vtkfigFrame.h"
#include "vtkfigCommunicator.h"
#include "vtkfigSurf2D.h"
#include "vtkfigScalarView.h"
#include "vtkfigVectorView.h"
#include "vtkfigGridView.h"
#include "vtkfigXYPlot.h"
#include "vtkfigMainThread.h"
#include "config.h"



namespace vtkfig
{

  namespace internals
  {
    ///
    /// Client side visualization setup
    ///
    class Client
    {
      
      int debug_level=0;
      int port=35000;
      std::string remotecmd;
      std::string hostname;
      int timeout=1000;

      bool help_switch=false;
      bool have_hostname=false;
      bool use_ssh=false;
      int iarg=1;
      bool remoteswitch=false;
      bool via=false;
      std::string via_hostname;
      vtkSmartPointer<Communicator> communicator;
    
    public:
      Client(int argc, const char * argv[])
      {
      
        MainThread::CreateMainThread();
        char *debug_string=getenv("VTKFIG_DEBUG");
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
            timeout=atoi(argv[iarg+1]); 
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

          if ((
                strcmp(argv[iarg],"-h")==0||
                strcmp(argv[iarg],"--help")==0
                )
              && (iarg+1<argc)) 
          {
            iarg+=1; 
            help_switch=true;
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

        if (help_switch || !remoteswitch)
        {
          std::cout << helpstring;
          exit(1);
        }
        
        if (remoteswitch)
        {
          std::string systemcmd;
          if (use_ssh)
          {
            cout << "vtkfig-exec: Connecting via ssh" << endl;
            systemcmd.append("ssh -q ");
          
            if (via)
            {
              cout << "vtkfig-exec: Connecting via " << via_hostname <<  endl;

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
          std::this_thread::sleep_for (std::chrono::milliseconds(timeout));
        }   
      
        communicator=vtkSmartPointer<Communicator>::New();
        communicator->SetReportErrors(0);
        communicator->client_connect_num_retry=2;
      
        int twait=10;
        int tfac=2;
        int iretry=0;
        int nretry=10;
        int rc=0;
      
        while (iretry<nretry)
        {
          cout << "vtkfig-exec: Client connecting to "<< hostname << ":" << port << "..." << endl;
          rc=communicator->ClientConnect(hostname.c_str(),port);
          if (rc) break;
          std::this_thread::sleep_for (std::chrono::milliseconds(twait));
          iretry++;
          twait*=tfac;
        }
      
        if (rc)
          cout << "vtkfig-exec: Client connected to "<< hostname << ":" << port << endl;
        else
        {

          cout << "vtkfig-exec: Client failed to connect to "<< hostname << ":" << port << endl;
          throw std::runtime_error("Giving up");
        }
      
      }

      /// Client receive-and-render loop
      int spin()
      {
        while (1)
        {
          Communicator::Command cmd;
          Frame* frame;
          int framenum=-1;        
        
//          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
          communicator->ReceiveCommand(cmd);
          communicator->ReceiveInt(framenum);
          if (debug_level>0) 
            cout << "vtkfig-exec: received: " << static_cast<int>(cmd) << " frame: " << framenum <<  " ";
        
          if (framenum>=0)
            frame=MainThread::mainthread->framemap[framenum];
        
          switch(cmd)
          {
          
          case Communicator::Command::Dummy:
          {
            if (debug_level>0)
              cout << " Dummy" << endl;
          }
          break;
        
          case Communicator::Command::Clear:
          {
            if (debug_level>0)
              cout << " clear" << endl;
          }
          break;
        
          case Communicator::Command::String:
          {
            std::string s;
            communicator->ReceiveString(s);
          
            if (debug_level>0)
              cout << " String: " <<  s << endl;
          }
          break;
        
          case Communicator::Command::Exit:
          {
            cout << " Exit by request" << endl;
            return 0;
          }
          break;
          
          case Communicator::Command::MainThreadAddFrame:
          {
            frame=new vtkfig::Frame();
            
            if (debug_level>0)
              cout << " New frame" << endl;
          }
          break;

          case Communicator::Command::FrameLayout:
          {
            int nX, nY;
            communicator->ReceiveInt(nX);
            communicator->ReceiveInt(nY);
            frame->SetLayout(nX,nY);
          
            if (debug_level>0)
              cout << " frame layout " << nX << " " << nY << endl;
          }
          break;

          case Communicator::Command::MainThreadRemoveFrame:
          {
            MainThread::mainthread->RemoveFrame(MainThread::mainthread->framemap[framenum]);
            if (debug_level>0)
              cout << " Remove Frame" << endl;
            
          }
          break;
        
          case Communicator::Command::FrameAddFigure:
          {
            std::string figtype;
            communicator->ReceiveString(figtype);
            int ipos;
            communicator->ReceiveInt(ipos);
          
            if (figtype=="Surf2D")
            {
              auto figure=new vtkfig::Surf2D();
              frame->AddFigure(figure,ipos);
              if (debug_level>0)
                cout << " Add Surf2d ipos= " << ipos << endl;
            }
            else if (figtype=="ScalarView")
            {
              auto figure=new vtkfig::ScalarView();
              frame->AddFigure(figure,ipos);
              if (debug_level>0)
                cout << " Add ScalarView ipos= " << ipos  << endl;
            }
            else if (figtype=="VectorView")
            {
              auto figure=new vtkfig::VectorView();
              frame->AddFigure(figure,ipos);
              if (debug_level>0)
                cout << " Add VectorView ipos= " << ipos  << endl;
            }
            else if (figtype=="GridView")
            {
              auto figure=new vtkfig::GridView();
              frame->AddFigure(figure,ipos);
              if (debug_level>0)
                cout << " Add GridView ipos= " << ipos  << endl;
            }
            else if (figtype=="XYPlot")
            {
              auto figure=new vtkfig::XYPlot();
              frame->AddFigure(figure,ipos);
              if (debug_level>0)
                cout << " Add XYPlot  ipos= " << ipos  << endl;
            }
            else
            {
              cout << " Communication error addfig: type"<< figtype << endl;
              return 0;
            }
          }
          break;
        
          case Communicator::Command::MainThreadShow:
          {
            if (debug_level>0)
              cout << " MainThreadShow" << endl;
            for (auto & framepair: MainThread::mainthread->framemap)
            {
              for (auto figure: framepair.second->figures)
              {
                figure->ClientMTReceiveData(communicator);
              }
            }
            MainThread::mainthread->Show();
          }
          break;
        
          case Communicator::Command::FrameActiveSubFrame:
          {
            assert(frame);
            int ipos;
            communicator->ReceiveInt(ipos);
            if (debug_level>0)
              cout << " SetActiveSubframe ipos:"<< ipos << endl;
            frame->SetActiveSubFrame(ipos);
          }
          break;

          case Communicator::Command::FrameSize:
          {
            int x,y;
            communicator->ReceiveInt(x);
            communicator->ReceiveInt(y);
            if (debug_level>0)
              cout << " FrameSize: " << x  << " " << y << endl;
            frame->SetSize(x,y);
          }
          break;
        
          case Communicator::Command::FramePosition:
          {
            int x,y;
            communicator->ReceiveInt(x);
            communicator->ReceiveInt(y);
            if (debug_level>0)
              cout << " FramePosition: " << x  << " " << y << endl;
            frame->SetPosition(x,y);
          }
          break;

          case Communicator::Command::FrameTitle:
          {
            std::string title;
            communicator->ReceiveString(title);
            if (debug_level>0)
              cout << " FrameTitle: " << title << endl;
            frame->SetFrameTitle(title);
          }
          break;
        
          case Communicator::Command::WindowTitle:
          {
            std::string title;
            communicator->ReceiveString(title);
            if (debug_level>0)
              cout << " WindowTitle: " << title << endl;
            frame->SetWindowTitle(title);
          }
          break;
        
          case Communicator::Command::FrameDump:
          {
            std::string fname;
            communicator->ReceiveString(fname);
            if (debug_level>0)
              cout << " FrameDump: " << fname << endl;
            frame->WritePNG(fname);
          }
          break;

          case Communicator::Command::FrameLinkCamera:
          {
            int thisframepos;
            int otherframenum;
            int otherframepos;
            communicator->ReceiveInt(thisframepos);
            communicator->ReceiveInt(otherframenum);
            communicator->ReceiveInt(otherframepos);
            if (debug_level>0)
              cout << " FrameLinkCamera: "<< endl;


            auto otherframe=MainThread::mainthread->framemap[otherframenum];
            frame->LinkCamera(
              thisframepos,
              otherframe,
              otherframepos);
          }
          break;

        
          case Communicator::Command::MainThreadTerminate:
          {
            if (debug_level>0)
              cout << " client termination" << endl;
            MainThread::mainthread->Terminate();
            return 0;
          }
          break;

        
          default:
          {
            throw std::runtime_error("vtkfig-exec: wrong command on client");
            return 0;
          }
        
          }
        
        }
      
      }
    };
  
  }  

}



int main(int argc, const char * argv[])
{
  
  vtkfig::internals::Client client(argc,argv);
  return client.spin();
}
