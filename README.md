vtkfig
======

[VTK](http://vtk.org)  based C++ graphics library for plotting and for data on rectilinear and unstructured grids with an flexible and easy to use API.

## Features

- Standard views for scalars, vectors and grids in 2D, 3D:
  - isolines, filled contours
  - plane cuts, isosurfaces
  - quiver plots, stream ribbons

- Duck typing based interface for data input in C++ codes

- Separate  rendering thread  allowing for  handling of  changing data   managed by the vtkfig::Frame class

- Extensible by implementing derived classes  containing  vtk rendering pipelines from vtkfig::Figure

- Memory management via `std::shared_ptr`

- (Emerging) C API

- (Emerging) companion Julia package [VTKFig.jl](https://github.com/j-fu/VTKFig.jl)

- Experimental client-server communication for remote execution

- 3-Clause BSD License ([same as vtk](http://www.vtk.org/licensing/))


## Further information
- [Documentation](https://www.wias-berlin.de/people/fuhrmann/vtkfig/html/index.html)
- [Source](https://github.com/j-fu/vtkfig)
- [Installation](doc/installation.md) 
- [Usage](doc/usage.md)  notes
- [Architecture](doc/architecture.md)  notes
- [Changes](doc/changes.md)
- Some notes on [performance](doc/performance.md) 


## Contributors

vtkfig started from [matplot](http://www.csc.kth.se/~dag/matplot_20091021.tar.gz) by Dag Lindbo found on [http://na-wiki.csc.kth.se/mediawiki/index.php/Mat_plot_vtk](http://na-wiki.csc.kth.se/mediawiki/index.php/Mat_plot_vtk). Numerous online postings by community members, the VTK examples, the doxygen documentation of VTK and sometimes its source code were helpful for developing this code.

