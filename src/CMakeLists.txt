function (add_vtkfig_binary  BINARY)
  add_executable(${BINARY} ${BINARY}.cxx)
  target_link_libraries(${BINARY} vtkfig ${VTK_LIBRARIES})
  target_include_directories(vtkfig SYSTEM PUBLIC ${VTK_INCLUDE_DIRS})
  set_property(TARGET ${BINARY} PROPERTY CXX_STANDARD 11)
  install(TARGETS ${BINARY} DESTINATION bin EXPORT ${VTKFIG_EXPORT_NAME})
endfunction()


add_library(vtkfig SHARED
  vtkfigThread.cxx
  vtkfigCommunicator.cxx
  vtkfigInteractorStyle.cxx
  vtkfigTimerCallback.cxx
  vtkfigFrame.cxx
  vtkfigFigure.cxx
  vtkfigDataSet.cxx
  vtkfigTools.cxx
  vtkfigGLScene.cxx
  vtkfigScalarView.cxx  
  vtkfigGridView.cxx  
  vtkfigVectorView.cxx  
  vtkfigXYPlot.cxx
  vtkfigCAPI.cxx
  vtkfigClient.cxx
  vtkfigSurf2D.cxx
#  vtkfigChartXY.cxx
  )

target_link_libraries(vtkfig  PUBLIC ${VTK_LIBRARIES})
target_include_directories(vtkfig SYSTEM PUBLIC ${VTK_INCLUDE_DIRS})
set_target_properties(vtkfig PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_property(TARGET vtkfig PROPERTY CXX_STANDARD 11)
install(TARGETS vtkfig DESTINATION lib EXPORT ${VTKFIG_EXPORT_NAME})


if(VTKFIG_BUILD_BINARIES)
  add_vtkfig_binary(vtkfig-exec)
  add_vtkfig_binary(vtkfig-openglinfo)
  add_vtkfig_binary(vtkfig-view-xyzrect)
endif()

