# SEE https://cmake.org/Wiki/CMake/Tutorials/How_to_create_a_ProjectConfig.cmake_file
cmake_minimum_required(VERSION 3.0)

PROJECT(VTKFIG C CXX)

set(VTKFIG_MAJOR_VERSION 0)
set(VTKFIG_MINOR_VERSION 6)
set(VTKFIG_PATCH_VERSION 0)
set(VTKFIG_VERSION    ${VTKFIG_MAJOR_VERSION}.${VTKFIG_MINOR_VERSION}.${VTKFIG_PATCH_VERSION})




# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH  "Installation directory for header files")
if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR CMake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/CMake/vtkfig)
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH  "Installation directory for CMake files")

 
# Make relative paths absolute (needed later on)
foreach(p LIB BIN INCLUDE CMAKE)
  set(var INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()


######################################
find_package(Threads REQUIRED)
find_package(VTK REQUIRED)

message(STATUS "VTK_USE_FILE= ${VTK_USE_FILE}")
include(${VTK_USE_FILE})


set(CMAKE_CXX_STANDARD 11)
include_directories(include)
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(examples)


######################################

file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}"   "${INSTALL_INCLUDE_DIR}")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(TARGETS vtkfig-exec vtkfig  FILE "${PROJECT_BINARY_DIR}/VTKFIGTargets.cmake")
export(PACKAGE VTKFIG)

set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include")
configure_file(cmake/VTKFIGConfig.cmake.in   "${PROJECT_BINARY_DIR}/VTKFIGConfig.cmake" @ONLY)
configure_file(cmake/VTKFIGConfigVersion.cmake.in  "${PROJECT_BINARY_DIR}/VTKFIGConfigVersion.cmake" @ONLY)
 




# ... for the install tree
#set(CONF_INCLUDE_DIRS "\${VTKFIG_CMAKE_DIR}/${REL_INCLUDE_DIR}")
set(CONF_INCLUDE_DIRS "\${CMAKE_CURRENT_LIST_DIR}/${REL_INCLUDE_DIR}/vtkfig")
configure_file(cmake/VTKFIGConfig.cmake.in   "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/VTKFIGConfig.cmake" @ONLY)
configure_file(cmake/VTKFIGConfigVersion.cmake.in  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/VTKFIGConfigVersion.cmake" @ONLY)

# Install the VTKFIGConfig.cmake and VTKFIGConfigVersion.cmake
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/VTKFIGConfig.cmake"
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/VTKFIGConfigVersion.cmake"
   DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)
 
# Install the export set for use with the install-tree
install(EXPORT VTKFIGTargets DESTINATION  "${INSTALL_CMAKE_DIR}" COMPONENT dev)


