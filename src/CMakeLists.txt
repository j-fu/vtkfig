function (add_vtkfig_binary  BINARY)
  add_executable(${BINARY} ${BINARY}.cxx)
  target_link_libraries(${BINARY} vtkfig ${VTK_LIBRARIES})
  set_property(TARGET ${BINARY} PROPERTY CXX_STANDARD 11)
  install(TARGETS ${BINARY} DESTINATION bin EXPORT ${VTKFIG_EXPORT_NAME})
endfunction()


add_library(vtkfig SHARED
  vtkfigFrame.cxx
  vtkfigFigure.cxx
  vtkfigDataSet.cxx
  vtkfigMainThread.cxx
  vtkfigCommunicator.cxx
  vtkfigTools.cxx
  vtkfigScalarView.cxx  
  vtkfigGridView.cxx  
  vtkfigVectorView.cxx  
  vtkfigXYPlot.cxx
  vtkfigCAPI.cxx
  
  vtkfigChartXY.cxx
  vtkfigSurf2D.cxx
  )
target_link_libraries(vtkfig vtkfig ${VTK_LIBRARIES})


###### target_link_libraries(vtkfig PUBLIC ${VTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
#target_link_libraries(vtkfig vtklib)
set_target_properties(vtkfig PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_property(TARGET vtkfig PROPERTY CXX_STANDARD 11)
install(TARGETS vtkfig DESTINATION lib EXPORT ${VTKFIG_EXPORT_NAME})

add_vtkfig_binary(vtkfig-exec)
add_vtkfig_binary(vtkfig-openglinfo)
add_vtkfig_binary(vtkfig-view-xyzrect)

