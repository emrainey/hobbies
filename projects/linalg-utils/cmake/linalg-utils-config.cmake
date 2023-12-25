# This file is meant to be installed then "found" via find_package()
include(CMakeFindDependencyMacro)
find_dependency(basal)
find_dependency(units_of_measure)
find_dependency(fourcc)
find_dependency(geometry)
find_dependency(linalg)
file(RELATIVE_PATH _var ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_FILE})
string(REPLACE "-config.cmake" "" _var "${_var}")
message(STATUS ">>> Found ${_var} at ${CMAKE_CURRENT_LIST_DIR}")
include("${CMAKE_CURRENT_LIST_DIR}/${_var}-Targets.cmake")