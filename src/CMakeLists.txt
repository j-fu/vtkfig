add_library(vtkfig 
  vtkfigFrame.cxx
  vtkfigFigure.cxx
  vtkfigCommunicator.cxx
  vtkfigServerConnection.cxx
  vtkfigTools.cxx
  vtkfigXYPlot.cxx
  vtkfigContour2DBase.cxx  
  vtkfigContour2D.cxx  
  vtkfigSurf2D.cxx
  vtkfigQuiver2D.cxx
  vtkfigContour3D.cxx  
  vtkfigTriContour2D.cxx  
)

add_executable(vtkfig-exec MACOSX_BUNDLE vtkfig-exec.cxx)
target_link_libraries(vtkfig-exec vtkfig ${VTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})



set_property(TARGET vtkfig-exec PROPERTY CXX_STANDARD 11)
set_property(TARGET vtkfig PROPERTY CXX_STANDARD 11)

