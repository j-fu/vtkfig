

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

add_executable(example-custom MACOSX_BUNDLE example-custom.cxx )
target_link_libraries(example-custom vtkfig ${VTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_executable(example-custom1 MACOSX_BUNDLE example-custom1.cxx )
target_link_libraries(example-custom1 vtkfig ${VTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_executable(example-tricontour2d MACOSX_BUNDLE example-tricontour2d.cxx )
target_link_libraries(example-tricontour2d vtkfig ${VTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_executable(example-tetcontour3d MACOSX_BUNDLE example-tetcontour3d.cxx )
target_link_libraries(example-tetcontour3d vtkfig ${VTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

# add_executable(example-simplexcontour MACOSX_BUNDLE example-simplexcontour.cxx )
# target_link_libraries(example-simplexcontour vtkfig ${VTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_executable(example-multifig MACOSX_BUNDLE example-multifig.cxx )
target_link_libraries(example-multifig vtkfig ${VTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_executable(example-multiframe MACOSX_BUNDLE example-multiframe.cxx )
target_link_libraries(example-multiframe vtkfig ${VTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})


add_executable(example-chartxy MACOSX_BUNDLE example-chartxy.cxx )
target_link_libraries(example-chartxy vtkfig ${VTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})


add_executable(example-comm-client-server-vtk MACOSX_BUNDLE example-comm-client-server-vtk.cxx )
target_link_libraries(example-comm-client-server-vtk vtkfig ${VTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_executable(example-comm-client-server-vtkfig MACOSX_BUNDLE example-comm-client-server-vtkfig.cxx )
target_link_libraries(example-comm-client-server-vtkfig vtkfig ${VTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})




