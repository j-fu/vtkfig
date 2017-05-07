cmake_minimum_required(VERSION 3.0)

PROJECT(vtkfig)

find_package(Threads REQUIRED)
find_package(VTK REQUIRED)

message(STATUS "VTK_USE_FILE= ${VTK_USE_FILE}")
include(${VTK_USE_FILE})


set(CMAKE_CXX_STANDARD 11)
include_directories(include)
add_subdirectory(src)
add_subdirectory(examples)
