Changes
=======


## Changes 

- v0.16
  - reworked XYPlot: added grid lines + better symbol handling, cleaned API

- v0.15
  - Cleaned API: renamed vtkfig::SurfaceContour to vtkfig::ScalarView
  - Cleaned API: joined vtkfig::Quiver and  vtkfig::Stream to vtkfig::VectorView
  - Coloring of quiver arrows and streamlines by velocity

- v0.14
  - checked on mac
  - fixed scaling issues

- v0.13
  - overhauled range scaling for contour+quiver, should be ok now
  - asterisk key switches between single/multi view
  - leafing through figures by Prior/Next in single view 

- v0.12
  - region masking and  2D elevation  plot for vtkfig::SurfaceContour
  - vtkfig::Quiver does 3D
  - vtkfig::Stream does stream ribbons
  - transparent domain outline for 3D for all vtkfig::Figure descendants
  - check with vtk 6.3, 7.1
  - documentation overhaul
  
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
