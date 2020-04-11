install(FILES
  vtkfigFigure.h
  vtkfigFrame.h
  vtkfigDataSet.h
  vtkfigTools.h
  vtkfigScalarView.h
  vtkfigGridView.h
  vtkfigGLScene.h
  vtkfigVectorView.h
  vtkfigXYPlot.h
DESTINATION include/vtkfig)

add_subdirectory(internals)

