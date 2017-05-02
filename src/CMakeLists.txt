add_library(vtkfig 
  vtkfigFrame.cxx
  vtkfigCommunicator.cxx
  vtkfigFigure.cxx
  vtkfigTools.cxx
  vtkfigXYPlot.cxx
  vtkfigContour2D.cxx  
  vtkfigSurf2D.cxx
  vtkfigQuiver2D.cxx
  vtkfigContour3D.cxx  
  vtkfigTriContour2D.cxx  
)

set_property(TARGET vtkfig PROPERTY CXX_STANDARD 11)

