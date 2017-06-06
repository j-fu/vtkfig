vtkfig
======


## Aim

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

- 3-Clause BSD License ([same as vtk](http://www.vtk.org/licensing/))

## Further information
- [Changes](changes.md)
- [Installation](installation.md) 
- [Usage](usage.md)  notes
- Some notes on [performance](performance.md) 


## Contributors

vtkfig                           started                          from
[matplot](http://www.csc.kth.se/~dag/matplot_20091021.tar.gz)  by  Dag
Lindbo found on   

[http://na-wiki.csc.kth.se/mediawiki/index.php/Mat_plot_vtk](http://na-wiki.csc.kth.se/mediawiki/index.php/Mat_plot_vtk).


Numerous online postings  by community members, the  VTK examples, the
doxygen  documentation  of VTK  and  sometimes  its source  code  were
helpful for setting up this code.

