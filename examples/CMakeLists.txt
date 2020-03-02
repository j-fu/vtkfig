function (add_vtkfig_example  EXAMPLE)
  add_executable(${EXAMPLE} ${EXAMPLE}.cxx)
  add_dependencies(${EXAMPLE} vtkfig)
  target_link_libraries(${EXAMPLE} vtkfig ${VTK_LIBRARIES} m)
  set_property(TARGET ${EXAMPLE} PROPERTY CXX_STANDARD 11)
endfunction()


add_executable(example-capi example-capi.c)
add_dependencies(example-capi vtkfig)
target_link_libraries(example-capi vtkfig ${VTK_LIBRARIES} m)



add_vtkfig_example(example-xyplot)
add_vtkfig_example(example-simplexcontour2d)
add_vtkfig_example(example-simplexcontour3d)
add_vtkfig_example(example-rectcontour2d)
add_vtkfig_example(example-rectcontour3d)

add_vtkfig_example(example-simplexquiver2d)
add_vtkfig_example(example-simplexquiver3d)
add_vtkfig_example(example-rectquiver2d)
add_vtkfig_example(example-rectquiver3d)

add_vtkfig_example(example-rectstream2d)
add_vtkfig_example(example-rectstream3d)

add_vtkfig_example(example-multifig)
add_vtkfig_example(example-gridview2d)
add_vtkfig_example(example-gridview3d)

add_vtkfig_example(example-custom-vtkfig)
add_vtkfig_example(example-custom-vtk)
add_vtkfig_example(example-multiframe)

add_vtkfig_example(example-comm-client-server-vtk)
add_vtkfig_example(example-comm-client-server-vtkfig)
add_vtkfig_example(example-scene2d)

if(VTKFIG_DEVEL)
  add_subdirectory(devel)
endif()

