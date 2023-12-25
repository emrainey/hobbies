# This file is meant to be installed then "found" via find_package()
include(CMakeFindDependencyMacro)
file(RELATIVE_PATH _var ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_FILE})
string(REPLACE "-Targets.cmake" "" _var "${_var}")
message(STATUS ">>> Found ${_var} at ${CMAKE_CURRENT_LIST_DIR}")
include("${CMAKE_CURRENT_LIST_DIR}/${_var}-Targets.cmake")