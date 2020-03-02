VTK installation
===============

There are two ways to install vtk such that it works for vtkfig.

## VTK Development package

With high probability, linux distributions provide a package named `vtk-devel` or similar. Installation of this package should be sufficient for compiling vtkfig. Similar possibilities exist for homebrew on MacOSX.

## VTK Compilation 

For the compilation of VTK, consult the [corresponding page](https://vtk.org/Wiki/VTK/Configure_and_Build) at Kitware, the maintainer of VTK.

vtkfig doesn't use the full functionality of vtk, and it is possible to work with a subset. This has two advantages:

- reduced  compile time for vtk
- less transient linking clashes with other system libraris, notably mpi and hdf5

Here is the cmake command which allows to build just enough  of vtk for a working vtkfig:
````
cmake ..\
 -GNinja\
 -DCMAKE_BUILD_TYPE=Release\
 -DCMAKE_INSTALL_PREFIX=$HOME/local\
 -DCMAKE_C_FLAGS:STRING=-fPIC\
 -DCMAKE_CXX_FLAGS:STRING=-fPIC\
 -DBUILD_SHARED_LIBS:BOOL=OFF\
 -DVTK_USE_SYSTEM_LIBRARIES:BOOL=ON\
 -DVTK_USE_SYSTEM_PNG:BOOL=ON\
 -DVTK_USE_SYSTEM_OGG:BOOL=ON\
 -DVTK_USE_SYSTEM_THEORA:BOOL=ON\
 -DVTK_USE_SYSTEM_TIFF:BOOL=ON\
 -DVTK_USE_SYSTEM_ZLIB:BOOL=ON\
 -DVTK_Group_Rendering:BOOL=OFF\
 -DVTK_Group_StandAlone:BOOL=OFF\
 -DModule_vtkCommonCore:BOOL=ON\
 -DModule_vtkCommonDataModel:BOOL=ON\
 -DModule_vtkCommonExecutionModel:BOOL=ON\
 -DModule_vtkCommonMisc:BOOL=ON\
 -DModule_vtkCommonSystem:BOOL=ON\
 -DModule_vtkCommonMath:BOOL=ON\
 -DModule_vtkCommonTransforms:BOOL=ON\
 -DModule_vtkRenderingCore:BOOL=ON\
 -DModule_vtkRenderingContext2D:BOOL=ON\
 -DModule_vtkFiltersCore:BOOL=ON\
 -DModule_vtkFiltersGeneral:BOOL=ON\
 -DModule_vtkFiltersGeometry:BOOL=ON\
 -DModule_vtkFiltersExtraction:BOOL=ON\
 -DModule_vtkFiltersFlowPaths:BOOL=ON\
 -DModule_vtkInteractionWidgets:BOOL=ON\
 -DModule_vtkIOMovie:BOOL=ON\
 -DModule_vtkParallelCore:BOOL=ON\
 -DModule_vtkInteractionStyle:BOOL=ON
````

- This script assumes working in a subdirectory named e.g. `build` of the top level directory of the vtk source tree (otherwise, replace the `..` accordingly).
- It uses `ninja` instead of `make` which performs a much faster parallel build. After configuring just invoke `ninja && ninja install`.
- Install prefix is `$HOME/local`, this can be changed to any other directory on the `CMAKE_PREFIX_PATH` where the vtkfig `CMakeListe.txt` can find stuff.
- Build no shared libraries, this helps to link everything into one `libvtkfig.so`. Just change this setting if you feel it is inconvenient (this setting also responsible for the -fPIC linker flags).
- We try to use system libraries for png  etc. Usually, these libraries are available, and using them may avoid some transient linking clashes.
- All the other stuff is just the heuristic selection of a subset of vtk which keeps vtkfig working. Fear not, it is still >1700 source files large.