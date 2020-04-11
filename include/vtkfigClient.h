///
/// \file vtkfigClient.h
///

#include <thread>
#include <cassert>
#include <memory>
#include <vtkSmartPointer.h>

#include "config.h"

#include "vtkfigFrame.h"
#include "vtkfigSurf2D.h"
#include "vtkfigScalarView.h"
#include "vtkfigVectorView.h"
#include "vtkfigGridView.h"
#include "vtkfigXYPlot.h"

#include "internals/vtkfigThread.h"
#include "internals/vtkfigCommunicator.h"

namespace vtkfig
{

  namespace internals
  {
    ///
    /// Client side visualization setup
    ///
    class Client
    {

    public:
      ///
      ///  Help string for vtkfig-exec
      ///
      static constexpr const char* ExecHelp=R"(
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


    private:
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
      /// Client constructor
      Client(int argc, const char * argv[]);

      /// Client receive-and-render loop
      int spin();
        
    };
      
  }
}
