add_library(vtkfig 
  vtkfigFrame.cxx
  vtkfigCommunicator.cxx
  vtkfigFigure.cxx
  vtkfig.cxx
  vtkfigXYPlot.cxx
  vtkfigContour2D.cxx  
  vtkfigSurf2D.cxx
)

set_property(TARGET vtkfig PROPERTY CXX_STANDARD 11)

