# This file is meant to be installed then "found" via find_package()
include(CMakeFindDependencyMacro)
find_dependency(basal)
find_dependency(units_of_measure)
find_dependency(fourcc)
include("${CMAKE_CURRENT_LIST_DIR}/LinalgTargets.cmake")