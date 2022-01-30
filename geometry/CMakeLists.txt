cmake_minimum_required(VERSION 3.13)
project(geometry CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(GTest REQUIRED)
find_package(basal REQUIRED)
find_package(units_of_measure REQUIRED)
find_package(linalg REQUIRED)
enable_testing()

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
add_definitions(-march=native -mtune=native)
else()
message(ERROR "No other native format is yet supported!")
endif()

add_library(emrainey-geometry SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/source/extra_math.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/intersection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/line.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/plane.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/point.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/ray.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/sphere.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/vector.cpp
)
target_link_libraries(emrainey-geometry PUBLIC
    Basal::emrainey-basal
    UnitsOfMeasure::emrainey-uom
    Linalg::emrainey-linalg
)
target_include_directories(emrainey-geometry PUBLIC
    # Generated Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    # Build Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    # Install Tree
    $<INSTALL_INTERFACE:include>
)

#####################
# EXPORTS
#####################

# Declare installs and exports
install(TARGETS emrainey-geometry EXPORT GeometryTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(EXPORT GeometryTargets
    FILE GeometryTargets.cmake
    NAMESPACE Geometry::
    DESTINATION share/cmake/geometry
)
install(DIRECTORY include/geometry
    DESTINATION include
    PATTERN "*.hpp"
    PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
)
install(FILES "cmake/Findgeometry.cmake" "cmake/geometry-config.cmake"
    DESTINATION share/cmake/geometry
)
#####################
# Googletests
#####################

if (${GTest_FOUND})
    add_executable(gtest_geometry
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_extra.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_intersections.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_line.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_plane.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_ray.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_sphere.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_vector.cpp
    )
    target_include_directories(gtest_geometry SYSTEM PUBLIC ${GTEST_INCLUDE_DIRS})
    target_link_libraries(gtest_geometry ${GTEST_BOTH_LIBRARIES} emrainey-geometry)
    add_test(NAME gtest_geometry COMMAND gtest_geometry)
endif()

#####################
# Build Documentation
#####################

if (${Doxygen_FOUND})
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(emrainey-geometry-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/geometry
        COMMENT "Geometry Doxygen")
endif()