Installation
============

## Installation

Prerequisite is a C++11 compiler and  an installed development version of VTK.  So far, the code been  tested with VTK 7.1, VTK 8.1 and VTK 8.2.  With few #ifdefs, on still can make it running on  VTK 6.3.   Build uses the standard CMake way. E.g. to configure with clang, use

```` 
$ mkdir build
$ cd build
$ CXX=clang++ cmake -DCMAKE_INSTALL_PREFIX=<installprefix> ..
$ make

````

Versions  of CMake  prior to 3.02  do  not do  automatic configuration  for C++11, so in this case you need to issue

```` 
$ CXX=clang++ CXXFLAG=-std=c++11 cmake ..

````

You will find the compiled examples in `build/examples`. `find_package` from another project will find `vtkfig` build tree via the cmake registry. The shared library `libvtkfig.so` is created in `build/src` and can be copied to some directory on `LD_LIBRARY_PATH`,
where it can be found by the Julia package `VTKFig.jl`.

Alternatively, perform in the build directory
````
$ make install
````
to install the includes and libraries in a directory defined by `CMAKE_INSTALL_PREFIX`.


## Testing + running examples


### Check OpenGL support on your desktop/laptop:

````
$ build/src/vtkfig-openglinfo
````

If the output is very terse (without notes on OpenGL v2 support, z-buffers etc), you probably will be bound to
a slow software rendering path. Fixing this depends on your VTK installation and your graphics hardware.

### Run example code

````
$ build/examples/example-simplexcontour2d
````
Similarly, you should be able to run any other example in `build/examples`.


### Run example code in multiprocess mode

This feature is rather experimental.

````
$ src/vtkfig-exec localhost examples/example-simplexcontour2d
````
You can add the -ssh flag and replace localhost by another computer in the network.




## Usage in another project

Use the usual cmake mechanism. In `CMakeLists.txt`, add

````
find_package(VTKFIG 0.20 QUIET)
````
and add `VTKFIG_INCLUDES` and `VTKFIG_LIBRARIES` to corresponding variables.

In particular, these will draw in all necessary vtk dependencies. 

