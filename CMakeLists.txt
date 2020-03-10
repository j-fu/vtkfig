# See https://cmake.org/Wiki/CMake/Tutorials/How_to_create_a_ProjectConfig.cmake_file
# see also https://github.com/forexample/package-example
# https://cmake.org/Wiki/CMake/Tutorials/Exporting_and_Importing_Targets



cmake_minimum_required(VERSION 2.8.12)
cmake_policy(SET CMP0037 OLD) 

PROJECT(VTKFIG C CXX)

set(VTKFIG_MAJOR_VERSION 0)
set(VTKFIG_MINOR_VERSION 21)
set(VTKFIG_PATCH_VERSION 20191106)
set(VTKFIG_VERSION    ${VTKFIG_MAJOR_VERSION}.${VTKFIG_MINOR_VERSION}.${VTKFIG_PATCH_VERSION})

# Allow to specify -DVTKFIG_CONFIG=xxx  at cmake time on the 
# command line of cmake.
SET(VTKFIG_CONFIG "" CACHE STRING "VTKFIG  Config name")

# Define an export name. It seems to be a good idea 
# to that the default is just the pakage name.
# This export name can be used by find_packge() 
# in the NAMES section in order to select different
# configs registered at for the package
# It should be used for all install(TARGET ... EXPORT ${VTKFIG_EXPORT_NAME}

set(VTKFIG_EXPORT_NAME "VTKFIG${VTKFIG_CONFIG}")

SET(VTKFIG_DEVEL False CACHE BOOL "VTKFIG  Devlopment configuration")
SET(VTKFIG_WITH_EXAMPLES True CACHE BOOL "Compile with examples")



#################################################################################
#
# Package content
 
find_package(Threads REQUIRED)
find_package(VTK REQUIRED)
message(STATUS "VTK version: ${VTK_VERSION}")

if("${VTK_VERSION_MAJOR}" LESS 6)
  message(STATUS "Supported VTK major versions are 6,7,8")
  message(FATAL_ERROR "insufficient VTK version")
endif()

#  We need this since the update to vtk 8 on OpenSUSE 42.2
if(NOT ("${VTK_VERSION_MAJOR}" LESS 8))
  find_package(Qt4)
endif()

# BEGIN COPIED from UseVTK.cmake
# Update CMAKE_MODULE_PATH so includes work.
list(APPEND CMAKE_MODULE_PATH ${VTK_CMAKE_DIR})

# Add compiler flags needed to use VTK.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${VTK_REQUIRED_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${VTK_REQUIRED_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${VTK_REQUIRED_EXE_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${VTK_REQUIRED_SHARED_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${VTK_REQUIRED_MODULE_LINKER_FLAGS}")

# Add preprocessor definitions needed to use VTK.
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${VTK_DEFINITIONS})

# Add include directories needed to use VTK.
include_directories(${VTK_INCLUDE_DIRS})

# Add link directories needed to use VTK.
link_directories(${VTK_LIBRARY_DIRS})
# END COPIED from UseVTK.cmake

get_filename_component(VTK_CMAKE_DIR "${VTK_USE_FILE}" PATH)
message(STATUS "VTK Configuration:" ${VTK_CMAKE_DIR})

# Add include directories needed to use VTK.
include_directories(${VTK_INCLUDE_DIRS})

# Add link directories needed to use VTK.
#link_directories(${VTK_LIBRARY_DIRS})



set(CMAKE_CXX_STANDARD 11)
include_directories(include ${PROJECT_BINARY_DIR})
add_subdirectory(include)
add_subdirectory(src)

if("${VTFIG_WITH_EXAMPLES}")
  add_subdirectory(examples)
endif()



if (APPLE)
  set(CONFIG_APPLE 1)
else()
  set(CONFIG_APPLE 0)
endif()

configure_file(cmake/config.h.in   "${PROJECT_BINARY_DIR}/config.h" @ONLY)

find_package(Doxygen)
if (DOXYGEN_FOUND)
  configure_file(${PROJECT_SOURCE_DIR}/doc/Doxyfile.in   "${PROJECT_BINARY_DIR}/Doxyfile" @ONLY)
  # Somehow, doxygen seems to not recognise .md files properly from other
  # than their directory
  add_custom_target( dox 
    COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM )
endif()



# 
# Archive
# 
set(ZIP_FILE_NAME_ROOT  "vtkfig-${VTKFIG_VERSION}")

#
# What is the easy way to do this with CPack proper ?
#
add_custom_target(zip
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMAND   git archive --format=zip -o ${PROJECT_BINARY_DIR}/${ZIP_FILE_NAME_ROOT}.zip HEAD
  )


#
# Upload doc
#
set(MYWWW "/home/fuhrmann/Wias/www/fuhrmann")
add_custom_target(upload 
                 COMMAND rsync -avu html ${MYWWW}/staging/vtkfig
                 COMMAND cp -rp ${PROJECT_BINARY_DIR}/${ZIP_FILE_NAME_ROOT}.zip ${MYWWW}/staging/vtkfig
                 COMMAND make -C ${MYWWW}/sitesrc upload
                 DEPENDS dox zip
                 )


#include_directories(${PROJECT_BINARY_DIR})

########################################################
#
# Export the package for use from the build-tree.
#
# This registers the build-tree with a global CMake-registry,
# On unix, it sits under .cmake/packages. 
# find_packages()  via the NAMES section is able to distinguish between
# them.
#
# This stuff allows to access the current installation 
# from other packages. No need to "make export" whatsoever.
#
# Furthermore, the target export allows client packages to
# figure out if they need a recompile or not.

export(TARGETS vtkfig-exec vtkfig FILE "${PROJECT_BINARY_DIR}/${VTKFIG_EXPORT_NAME}Targets.cmake" EXPORT_LINK_INTERFACE_LIBRARIES)
export(PACKAGE ${VTKFIG_EXPORT_NAME})


# In order to work, we need to set up  the corresponding 
# Files in ${PROJECT_BINARY_DIR} which is the root of the
# build subdirectory


set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include")
configure_file(cmake/VTKFIGConfig.cmake.in   "${PROJECT_BINARY_DIR}/${VTKFIG_EXPORT_NAME}Config.cmake" @ONLY)
configure_file(cmake/VTKFIGConfigVersion.cmake.in  "${PROJECT_BINARY_DIR}/${VTKFIG_EXPORT_NAME}ConfigVersion.cmake" @ONLY)
 


########################################################
#
# Installation magic, still to be fully understod
# 
# Relevant only for make install. 
#
# Relocatable according to 
# https://cmake.org/Wiki/CMake/Tutorials/How_to_create_a_ProjectConfig.cmake_file
#
# It works from the same config.in files 
#  cmake/VTKFIGConfig.cmake.in 
#  cmake/VTKFIGConfigVersion.cmake.in 
# as the export, to be instantiated in another place in order
# to avoid clashes.

#
# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH  "Installation directory for header files")
if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR CMake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/cmake/vtkfig)
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH  "Installation directory for CMake files")

 
# Make relative paths absolute (needed later on)
foreach(p LIB BIN INCLUDE CMAKE)
  set(var INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()




file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}"   "${INSTALL_INCLUDE_DIR}")


set(CONF_INCLUDE_DIRS "\${CMAKE_CURRENT_LIST_DIR}/${REL_INCLUDE_DIR}/vtkfig")
list(APPEND CONF_INCLUDE_DIRS "\${CMAKE_CURRENT_LIST_DIR}/${REL_INCLUDE_DIR}")

configure_file(cmake/VTKFIGConfig.cmake.in   "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${VTKFIG_EXPORT_NAME}Config.cmake" @ONLY)
configure_file(cmake/VTKFIGConfigVersion.cmake.in  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${VTKFIG_EXPORT_NAME}ConfigVersion.cmake" @ONLY)

# Install the ${VTKFIG_EXPORT_NAME}Config.cmake and ${VTKFIG_EXPORT_NAME}ConfigVersion.cmake
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${VTKFIG_EXPORT_NAME}Config.cmake"
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${VTKFIG_EXPORT_NAME}ConfigVersion.cmake"
   DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)
 
# Install the export set for use with the install-tree
install(EXPORT ${VTKFIG_EXPORT_NAME} 
  DESTINATION  "${DEF_INSTALL_CMAKE_DIR}" 
  FILE "${VTKFIG_EXPORT_NAME}Targets.cmake"
  COMPONENT dev)

# .. and define an uninstall target.
add_custom_target(uninstall cat ${PROJECT_BINARY_DIR}/install_manifest.txt | xargs rm -fv)

