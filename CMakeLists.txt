

cmake_minimum_required(VERSION 3.0)

PROJECT(vtkfig)

find_package(VTK REQUIRED)

set(FLTK_PATH "../../usr/")
find_package(FLTK NO_DEFAULT_PATH PATHS ${FLTK_PATH})
include_directories(${FLTK_PATH}/include)




include(${VTK_USE_FILE})

add_executable(matplot2-example MACOSX_BUNDLE matplot2-example.cpp matplot2.cpp)
set_property(TARGET matplot2-example PROPERTY CXX_STANDARD 11)

if(VTK_LIBRARIES)
  target_link_libraries(matplot2-example ${VTK_LIBRARIES})
endif()


add_library(vtkfig vtkfig.cxx)
set_property(TARGET vtkfig PROPERTY CXX_STANDARD 11)

add_executable(example-xyplot MACOSX_BUNDLE example-xyplot.cxx )
set_property(TARGET example-xyplot PROPERTY CXX_STANDARD 11)
target_link_libraries(example-xyplot vtkfig ${VTK_LIBRARIES})


add_executable(example-contour2d MACOSX_BUNDLE example-contour2d.cxx )
set_property(TARGET example-contour2d PROPERTY CXX_STANDARD 11)
target_link_libraries(example-contour2d vtkfig ${VTK_LIBRARIES})

add_executable(example-surf2d MACOSX_BUNDLE example-surf2d.cxx )
set_property(TARGET example-surf2d PROPERTY CXX_STANDARD 11)
target_link_libraries(example-surf2d vtkfig ${VTK_LIBRARIES})

add_executable(example-quiver2d MACOSX_BUNDLE example-quiver2d.cxx )
set_property(TARGET example-quiver2d PROPERTY CXX_STANDARD 11)
target_link_libraries(example-quiver2d vtkfig ${VTK_LIBRARIES})

add_executable(example-contour3d MACOSX_BUNDLE example-contour3d.cxx )
set_property(TARGET example-contour3d PROPERTY CXX_STANDARD 11)
target_link_libraries(example-contour3d vtkfig ${VTK_LIBRARIES})




add_executable(ColoredElevationMap MACOSX_BUNDLE ColoredElevationMap.cxx)
if(VTK_LIBRARIES)
  target_link_libraries(ColoredElevationMap vtkfig ${VTK_LIBRARIES})
endif()

add_executable(LinePlot MACOSX_BUNDLE LinePlot.cxx)
if(VTK_LIBRARIES)
  target_link_libraries(LinePlot vtkfig ${VTK_LIBRARIES})
endif()


add_executable(Cone3 MACOSX_BUNDLE Cone3.cxx vtkFlRenderWindowInteractor.cxx)
if(VTK_LIBRARIES)
  target_link_libraries(Cone3 vtkfig ${VTK_LIBRARIES} vtkfig ${FLTK_LIBRARIES}  fltk fltk_gl dl)
endif()

add_executable(thread MACOSX_BUNDLE thread.cxx)
if(VTK_LIBRARIES)
  target_link_libraries(thread vtkfig ${VTK_LIBRARIES})
  set_property(TARGET thread PROPERTY CXX_STANDARD 11)
endif()
