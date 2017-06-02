Installation
============

## Installation

Prerequisite is a C++11 compiler and  an installed version of VTK.  So
far, the codee been  tested with VTK 6.3 and VTK  7.0.  Build uses the
standard CMake way. E.g. to configure with clang, use

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
