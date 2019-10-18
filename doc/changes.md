Changes
=======


## Changes
- tip
  - Removed reference based API as this cannot be handled well together with  the shared_ptr based stuff.
  - Refactored pointer handling: internally, nearly all interfaces now use shared_ptr or references
  - Moved to github due to Atlassian sunsetting mercurial support on bitbucket and compatibility
    to Julia world
  - Started C language API (mainly for calling from Julia)
  - Restructured documentation
  - Switched examples to reference based idiom due to better interaction with doxygen
  - dropped vtk6 support
  
- v0.20
  - Internal restructuring for changing grids: use vtkTrivialDataProducer instead of data object
    when constructing pipelines, detect  region number ranges with vminmax. No API change necessary.
  - Video recording via vtkOggTheoraWriter

- v0.19
  - use wireframe instead of vtkExtractEdges for grid view (much faster)
  - VTKFIG_DEVEL cache variabe to enable developing examples
  - fixed server-client examples
  - make install now fully functional
  - install(EXPORT) with relative directory
  
- v0.18
  - Adaptations for vtk8. Supported vtk versions are now
    6,7 and 8.

- v0.17
  - better handling of small scales:
     - introduced coordinate scale factor
     - internal storage in float
  - show blocked/nonblocked state, active figure
  - fixed streamline API, defaults
  - doxygen not anymore target in ALL
  - Do not use VTK_USE_FILE
  - Added setting of camera and view volume to API
  - Added rectangular grid data viewer vtkfig-view-xyzrect

- v0.16
  - reworked XYPlot: added grid lines + better symbol handling, cleaned API
  - 3D grid view
  - Boundary grid handling

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
