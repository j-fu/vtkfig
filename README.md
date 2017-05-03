vtkfig V0.2
============

The intention of  vtkfig is to provide a C++  graphics library with an API similar to that of matplotlib and which uses [vtk](http://vtk.org) for rendering.

## Features

- Mainly C++11
- Separate rendering thread allowing for easy handling of changing data  handeled by the vtkfig::Frame class
- Easy extension by implementing derived classes from vtkfig::Figure containing standard vtk rendering pipelines


## Build

Build uses the standard CMake way. E.g. to configure with clang, use

```` 
$ mkdir build
$ cd build
$ CXX=clang++ cmake ..
$ make

````

You will find the compiled examples in build/examples.



## Contributors

vtkfig started from [matplot](http://www.csc.kth.se/~dag/matplot_20091021.tar.gz) by Dag Lindbo found on [http://na-wiki.csc.kth.se/mediawiki/index.php/Mat_plot_vtk](http://na-wiki.csc.kth.se/mediawiki/index.php/Mat_plot_vtk).


Numerous online postings by community members and the VTK examples were extremely helpful for setting up this code.
