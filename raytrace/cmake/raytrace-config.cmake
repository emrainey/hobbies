# This file is meant to be installed then "found" via find_package()
include(CMakeFindDependencyMacro)
find_dependency(basal)
find_dependency(units_of_measure)
find_dependency(fourcc)
find_dependency(geometry)
find_dependency(linalg)
find_dependency(noise)
find_dependency(xmmt)
find_dependency(OpenMP)
include("${CMAKE_CURRENT_LIST_DIR}/RaytraceTargets.cmake")