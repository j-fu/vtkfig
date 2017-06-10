add_library(vtkfig 
  vtkfigFrame.cxx
  vtkfigFigure.cxx
  vtkfigDataSet.cxx
  vtkfigMainThread.cxx
  vtkfigCommunicator.cxx
  vtkfigTools.cxx
  vtkfigScalarView.cxx  
  vtkfigGridView.cxx  
  vtkfigQuiver.cxx  
  vtkfigStream.cxx  
  vtkfigXYPlot.cxx

  
  vtkfigChartXY.cxx
  vtkfigSurf2D.cxx
)

###### target_link_libraries(vtkfig PUBLIC ${VTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
#target_link_libraries(vtkfig vtklib)
set_target_properties(vtkfig PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_executable(vtkfig-exec MACOSX_BUNDLE vtkfig-exec.cxx)
target_link_libraries(vtkfig-exec vtkfig ${VTK_LIBRARIES})

add_executable(vtkfig-openglinfo MACOSX_BUNDLE vtkfig-openglinfo.cxx)
target_link_libraries(vtkfig-openglinfo ${VTK_LIBRARIES})

set_property(TARGET vtkfig-exec PROPERTY CXX_STANDARD 11)
set_property(TARGET vtkfig-openglinfo PROPERTY CXX_STANDARD 11)

set_property(TARGET vtkfig PROPERTY CXX_STANDARD 11)


install(TARGETS vtkfig DESTINATION lib EXPORT ${VTKFIG_EXPORT_NAME})
install(TARGETS vtkfig-exec DESTINATION bin EXPORT ${VTKFIG_EXPORT_NAME})
install(TARGETS vtkfig-openglinfo DESTINATION bin EXPORT ${VTKFIG_EXPORT_NAME})

