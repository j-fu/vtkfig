
## Server-client  communication

In build/src/vtkfig-exec is the  experimental driver for server-client
communication.  The examples ``example-xyplot`` and ``example-surf2d``
are equipped  with this  feature.  Here  is how to  use it  (we assume
working from  the ``build``  directory, ``port``  is an  optional port
number, ``vtkfigpath`` is  the path to the vtkfig  source directory on
the remote host ):

   - Run example locally

     ````
     $ examples/example-surf2d
     ````

   - Run example in client server mode on localhost

     ````
     $ src/vtkfig-exec -p port  localhost examples/example-surf2d
     ````


   - Run example on remote host ``remote``

     ````
     $ src/vtkfig-exec -p port -ssh  remote vtkfigpath/build/examples/example-surf2d
     ````

   - Run example on remote host ``remote`` with ssh tunnel via gateway  ``gate``

     ````
     $ src/vtkfig-exec -p port -via gate -ssh  remote vtkfigpath/build/examples/example-surf2d
     ````

Initial experiments (for example-surf2d) show the following perfomance comparison

| location  render path  system  card/connection       frame rate |
|-----------------------------------------------------------------|
| Local     direct       laptop  NVIDIA Quadro K610    43 fps     |
| Local     server       laptop  NVIDIA Quadro K610    40 fps     |
| Local     direct       minipc  Intel Skylake DT GT2  20 fps     |
| Local     server       minipc  Intel Skylake DT GT2  20 fps     |
|                                                                 |
| Remote    server       laptop  Ethernet/tunnel       22 fps     |
| Remote    server       laptop  Ethernet              20 fps     |
| Remote    server       minipc  Ethernet/tunnel       20 fps     |
| Remote    server       minipc  Ethernet              12 fps     |
| Remote    vnc/direct   laptop  Ethernet/tunnel       8 fps      |
| Remote    vnc/direct   laptop  Ethernet              8 fps      |
| Remote    vnc/direct   minipc  Ethernet/tunnel       8 fps      |
| Remote    vnc/direct   minipc  Ethernet              8 fps      |
| Remote    vnc/direct   laptop  WLAN/tunnel           8 fps      |
| Remote    vnc/direct   laptop  WLAN                  8 fps      |
| Remote    server       laptop  WLAN/tunnel           6 fps      |
| Remote    server       laptop  WLAN                  4 fps      |



A  preliminary  conclusion  tells  us   that  the  perfomance  of  the
client-server  render  path  strongly  depends on  the  speed  of  the
physical  connection.   Only  with  ethernet there  is  a  significant
advantage  over  standard  vnc   (where  software  rendering  ist  the
default).    A   solution   for   remote   rendering   might   be
TurboVNC+VirtualGL on servers with GPUs.
