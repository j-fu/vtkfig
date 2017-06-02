Changes
=======


## Changes 

- v0.11
  - vtkfig::DataSet now unifies unstructured and rectilinear grid data
  - some work on documentation

- v0.10
  - quiver 2D now scales right
  - arrow scale edit
  - reshuffle architecture: transform data instead of model view transform, otherwise
    quiver wouldn't scale right in anisotropic cases

- v0.9:
  - plane/isolevel edit

- v0.8: 
  - architecture reshuffle: separate data from figures
  - environment variable VTKFIG_MULTITHREADED controls single/multithread
    rendering. On the mac 0 is the default...
