Architecture
=============

The code can run in several modes: single threaded, multithreaded and in multiple processes


## Single threaded mode

This mode becomes active when setting the environment variable VTKFIG_MULTITHREADED to 0.

All code runs under the current user thread. Rendering and interaction events are processed
within the calls to vtkfig::Frame::Show resp. vtkfig::Frame::Interact. Outside of these
calls, the user interface is not responsive.

## Multi threaded mode

This mode is active  by default or when setting the environment variable VTKFIG_MULTITHREADED to 1.

vtkfig spawns a rendering thread which issues all commands necessary to set up and interact with
vtk rendering pipelines. At the same time, all interaction events are processed in this thread.
State updates from the user thread are handeld via mutex locks. 

## Multi process mode

This mode is activated when starting an executable using vtkfig via `vtkfig-exec` like this:

````
$ vtkfig-exec host example-command
````

For more elaborate information, see vtkfig::internals::Client::ExecHelp.

`host` can be `localhost` or another network host. `example-command` is run on this host. In
this case, the rendering thread of `example-command` sends all rendering information through
the network to the computer running `vtkfig-exec`, and rendering is performed on its screen.


