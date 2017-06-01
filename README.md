vtkfig V0.11
============

The intention of  vtkfig is to provide a C++  graphics library with an
API that is as easy to use as that of python/matplotlib and which uses
[vtk](http://vtk.org) for rendering.

## Features

- Duck typing based interface for data allows flexible use

- Separate  rendering thread  allowing for  handling of  changing data
  managed by the vtkfig::Frame class

- Standard views for 2D, 3D, grid

- Extensible by implementing derived classes from vtkfig::Figure using
  vtk rendering pipelines

- Experimental client-server communication for remote execution

- 3-Clause BSD License ([same as vtk](http://www.vtk.org/licensing/)

## News

- v0.11
  - vtkfig::DataSet now unifies unstructured and rectilinear grid data
  - some work on documentation

- v0.10 quiver 2D now scales right
  - arrow scale edit
  - reshuffle architecture: transform data instead of model view transform, otherwise
    quiver wouldn't scale right in anisotropic cases

- v0.9: plane/isolevel edit

- v0.8: 
  - architecture reshuffle: separate data from figures
  - environment variable VTKFIG_MULTITHREADED controls single/multithread
    rendering. On the mac 0 is the default...
 
## Build

Prerequisite is  an C++11  compiler and an  installed version  of VTK.
Nothing of  this code has  been tested with  versions of VTK  prior to
7.0.  Build uses the standard CMake way. E.g. to configure with clang,
use

```` 
$ mkdir build
$ cd build
$ CXX=clang++ cmake ..
$ make

````

Versions  of CMake  pre 3.02  do  not do  automatic configuration  for
C++11, so in this case you need to issue

```` 
$ CXX=clang++ CXXFLAG=-std=c++11 cmake ..

````


You will find the compiled examples in build/examples.


## Contributors

vtkfig                           started                          from
[matplot](http://www.csc.kth.se/~dag/matplot_20091021.tar.gz)  by  Dag
Lindbo                             found                            on
[http://na-wiki.csc.kth.se/mediawiki/index.php/Mat_plot_vtk](http://na-wiki.csc.kth.se/mediawiki/index.php/Mat_plot_vtk).


Numerous online postings  by community members, the  VTK examples, the
doxygen  documentation  of VTK  and  sometimes  its source  code  were
helpful for setting up this code.

