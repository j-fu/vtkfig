add_library(vtkfig SHARED
  vtkfigFrame.cxx
  vtkfigFigure.cxx
  vtkfigMainThread.cxx
  vtkfigCommunicator.cxx
  vtkfigTools.cxx
  vtkfigContour2DBase.cxx  
  vtkfigContour3DBase.cxx  
 
  vtkfigXYPlot.cxx
  vtkfigChartXY.cxx
  vtkfigContour2D.cxx  
  vtkfigSurf2D.cxx
  vtkfigQuiver2D.cxx
  vtkfigContour3D.cxx  
  vtkfigTriContour2D.cxx  
  vtkfigTetContour3D.cxx  
)

add_executable(vtkfig-exec MACOSX_BUNDLE vtkfig-exec.cxx)
target_link_libraries(vtkfig-exec vtkfig ${VTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})


set_property(TARGET vtkfig-exec PROPERTY CXX_STANDARD 11)

set_property(TARGET vtkfig PROPERTY CXX_STANDARD 11)


install(TARGETS vtkfig DESTINATION lib EXPORT ${VTKFIG_EXPORT_NAME})
install(TARGETS vtkfig-exec DESTINATION bin EXPORT ${VTKFIG_EXPORT_NAME})

