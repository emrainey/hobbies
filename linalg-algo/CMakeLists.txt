cmake_minimum_required(VERSION 3.13)
project(linalg-algo CXX)
set(CMAKE_CXX_STANDARD 17)
enable_testing()

if (USE_CONAN)
include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
endif()

find_package(OpenCV REQUIRED)
find_package(GTest REQUIRED)
find_package(basal REQUIRED)
find_package(units_of_measure REQUIRED)
find_package(fourcc REQUIRED)
find_package(linalg REQUIRED)
find_package(linalg-utils REQUIRED)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
add_definitions(-march=native -mtune=native)
else()
message(ERROR "No other native format is yet supported!")
endif()

add_library(emrainey-linalg-algo SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/source/kmeans.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/dkf.cpp
)
target_link_libraries(emrainey-linalg-algo PUBLIC
    Geometry::emrainey-geometry
    LinalgUtils::emrainey-linalg-utils
)
target_include_directories(emrainey-linalg-algo
    PUBLIC
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
install(TARGETS emrainey-linalg-algo EXPORT LinalgAlgoTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(EXPORT LinalgAlgoTargets
    FILE LinalgAlgoTargets.cmake
    NAMESPACE LinalgAlgo::
    DESTINATION share/cmake/linalg-algo
)
install(DIRECTORY include/linalg
    DESTINATION include
    PATTERN "*.hpp"
    PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
)
install(FILES "cmake/Findlinalg-algo.cmake" "cmake/linalg-algo-config.cmake"
    DESTINATION share/cmake/linalg-algo
)
#####################
# Demos
#####################

if (${OpenCV_FOUND})
    add_executable(demo_kmeans ${CMAKE_CURRENT_SOURCE_DIR}/demo/demo_kmeans.cpp)
    target_include_directories(demo_kmeans PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(demo_kmeans emrainey-linalg-algo)
endif()

#####################
# Build Documentation
#####################

if (${Doxygen_FOUND})
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(emrainey-linalg-algo-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/linalg
        COMMENT "Linalg + Algo Doxygen")
endif()