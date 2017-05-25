function (add_vtkfig_example  EXAMPLE)
  add_executable(${EXAMPLE} MACOSX_BUNDLE ${EXAMPLE}.cxx)
  target_link_libraries(${EXAMPLE} vtkfig ${VTK_LIBRARIES})
endfunction()


add_vtkfig_example(example-xyplot)
add_vtkfig_example(example-surf2d)
add_vtkfig_example(example-quiver2d)
add_vtkfig_example(example-custom)
add_vtkfig_example(example-custom1)
add_vtkfig_example(example-simplexcontour2d)
add_vtkfig_example(example-simplexcontour3d)
add_vtkfig_example(example-rectcontour2d)
add_vtkfig_example(example-rectcontour3d)
add_vtkfig_example(example-chartxy)
add_vtkfig_example(example-comm-client-server-vtk)
add_vtkfig_example(example-comm-client-server-vtkfig)


# re-establish
# add_vtkfig_example(example-multifig)
# add_vtkfig_example(example-multiframe)

