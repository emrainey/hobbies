cmake_minimum_required(VERSION 3.20.0)
project(hobbies CXX)

#set(CMAKE_PREFIX_PATH "install")
#set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*)

# lists which things are subprojects and which are not
set(AS_SUBPROJECTS basal units_of_measure fourcc linalg geometry)
# xmmt linalg neuralnet noise raytrace htm)

function(print_package_variables PREFIX)
    message(STATUS "${PREFIX}_FOUND=${${PREFIX}_FOUND}")
    message(STATUS "${PREFIX}_INCLUDE_DIRS=${${PREFIX}_INCLUDE_DIRS}")
    message(STATUS "${PREFIX}_LIBRARIES=${${PREFIX}_LIBRARIES}")
endfunction()

# Append all the subproject cmake folders so that they can be "found"
foreach(pkg ${AS_SUBPROJECTS})
    list(APPEND CMAKE_PREFIX_PATH "${pkg}/cmake")
    list(APPEND CMAKE_MODULE_PATH "${pkg}/cmake")
endforeach()
message(STATUS "CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}")
message(STATUS "CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}")

# This allows subprojects to use the "find_package()" function and for it to work transparently for subprojects
macro(find_package)
    message(STATUS "Is ${ARGV0} in ${AS_SUBPROJECTS}")
    if (NOT "${ARGV0}" IN_LIST AS_SUBPROJECTS)
        _find_package(${ARGV})
    endif()
endmacro()

# Add each subproject
foreach(pkg ${AS_SUBPROJECTS})
    add_subdirectory(${pkg})
endforeach()
