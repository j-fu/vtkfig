/// \file vtkfig-exec.cxx
///
/// Compiles to executable performing server-client communcation
///
/// Starts on local computer and spawns user program running on remote host sending
/// rendering data to local host.
/// 

#include <string>
#include "config.h"
#include "vtkfigClient.h"




int main(int argc, const char * argv[])
{
  
  vtkfig::internals::Client client(argc,argv);
  return client.spin();
}
