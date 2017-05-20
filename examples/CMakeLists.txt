function (add_vtkfig_example  EXAMPLE)
  add_executable(${EXAMPLE} MACOSX_BUNDLE ${EXAMPLE}.cxx)
  target_link_libraries(${EXAMPLE} vtkfig ${VTK_LIBRARIES})
endfunction()


add_vtkfig_example(example-xyplot)
add_vtkfig_example(example-contour2d)
add_vtkfig_example(example-surf2d)
add_vtkfig_example(example-quiver2d)
add_vtkfig_example(example-contour3d)
add_vtkfig_example(example-custom)
add_vtkfig_example(example-custom1)
add_vtkfig_example(example-tricontour2d)
add_vtkfig_example(example-tetcontour3d)
add_vtkfig_example(example-multifig)
add_vtkfig_example(example-multiframe)
add_vtkfig_example(example-chartxy)
add_vtkfig_example(example-comm-client-server-vtk)
add_vtkfig_example(example-comm-client-server-vtkfig)



