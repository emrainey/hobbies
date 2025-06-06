cmake_minimum_required(VERSION 3.31.0)
project(hobbies
    VERSION 1.0.0
    DESCRIPTION "A repo full of hobby projects"
    LANGUAGES CXX
)
include(CMakeDependentOption)

option(BUILD_SHARED_LIBS "Builds libraries as dynamic objects" OFF)
option(USE_COVERAGE "Enable Coverage" OFF)
cmake_dependent_option(USE_PROFILING "Enable Profiling" OFF "USE_COVERAGE" ON)

# Build Everything with C++17 and full native optimizations
set(CMAKE_CXX_STANDARD 17)
enable_testing()

add_subdirectory(projects/basal)
add_subdirectory(projects/units_of_measure)
add_subdirectory(projects/fourcc)
add_subdirectory(projects/xmmt)
add_subdirectory(projects/noise)
add_subdirectory(projects/linalg)
add_subdirectory(projects/geometry)
add_subdirectory(projects/linalg-utils)
add_subdirectory(projects/linalg-algo)
add_subdirectory(projects/vision)
add_subdirectory(projects/raytrace)
add_subdirectory(projects/neuralnet)
add_subdirectory(projects/htm)
add_subdirectory(projects/doorgame)
add_subdirectory(projects/audio)
add_subdirectory(projects/pyhobbies)