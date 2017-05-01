add_library(vtkfig 
  vtkfigFrame.cxx
  vtkfigCommunicator.cxx
  vtkfigFigure.cxx
  vtkfig.cxx
  vtkfigXYPlot.cxx
)

set_property(TARGET vtkfig PROPERTY CXX_STANDARD 11)

