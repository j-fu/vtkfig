

add_executable(example-xyplot MACOSX_BUNDLE example-xyplot.cxx )
target_link_libraries(example-xyplot vtkfig ${VTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_executable(example-contour2d MACOSX_BUNDLE example-contour2d.cxx )
target_link_libraries(example-contour2d vtkfig ${VTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_executable(example-surf2d MACOSX_BUNDLE example-surf2d.cxx )
target_link_libraries(example-surf2d vtkfig ${VTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_executable(example-quiver2d MACOSX_BUNDLE example-quiver2d.cxx )
target_link_libraries(example-quiver2d vtkfig ${VTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_executable(example-contour3d MACOSX_BUNDLE example-contour3d.cxx )
target_link_libraries(example-contour3d vtkfig ${VTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_executable(example-tricontour2d MACOSX_BUNDLE example-tricontour2d.cxx )
target_link_libraries(example-tricontour2d vtkfig ${VTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_executable(example-comm-client-server MACOSX_BUNDLE example-comm-client-server.cxx )
target_link_libraries(example-comm-client-server vtkfig ${VTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})


