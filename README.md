vtkfig
======


## Aim

The intention of  vtkfig is to provide a C++  graphics library with an
API     that    is     as     easy    to     use     as    that     of
[python/matplotlib](https://matplotlib.org/)     and    which     uses
[vtk](http://vtk.org) for fast rendering.

## Features

- Standard views for scalars, vectors and grids in 2D, 3D:
    - isolines, filled contours
    - plane cuts, isosurfaces
    - quiver plots, stream ribbons

- Duck typing based interface for data input

- Separate  rendering thread  allowing for  handling of  changing data
  managed by the vtkfig::Frame class

- Extensible by implementing derived classes  containing
  vtk rendering pipelines from vtkfig::Figure

- Experimental client-server communication for remote execution

- 3-Clause BSD License ([same as vtk](http://www.vtk.org/licensing/))


## Further information
- [Changes](doc/changes.md)
- [Installation](doc/installation.md) 
- [Usage](doc/usage.md)  notes
- Some notes on [performance](doc/performance.md) 


## Contributors

vtkfig                           started                          from
[matplot](http://www.csc.kth.se/~dag/matplot_20091021.tar.gz)  by  Dag
Lindbo found on   

[http://na-wiki.csc.kth.se/mediawiki/index.php/Mat_plot_vtk](http://na-wiki.csc.kth.se/mediawiki/index.php/Mat_plot_vtk).


Numerous online postings  by community members, the  VTK examples, the
doxygen  documentation  of VTK  and  sometimes  its source  code  were
helpful for setting up this code.

