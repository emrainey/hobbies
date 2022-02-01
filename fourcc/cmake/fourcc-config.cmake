# This file is meant to be installed then "found" via find_package()
include(CMakeFindDependencyMacro)
find_dependency(basal)
include("${CMAKE_CURRENT_LIST_DIR}/FourccTargets.cmake")