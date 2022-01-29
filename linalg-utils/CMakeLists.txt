cmake_minimum_required(VERSION 3.13)
project(linalg CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package(GTest REQUIRED)
find_package(basal REQUIRED)
find_package(units_of_measure REQUIRED)
find_package(fourcc REQUIRED)
find_package(linalg REQUIRED)
find_package(geometry REQUIRED)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
add_definitions(-march=native -mtune=native)
else()
message(ERROR "No other native format is yet supported!")
endif()


if (${OpenCV_FOUND})
add_library(emrainey-linalg-utils SHARED)
target_sources(emrainey-linalg-utils
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source/opencv.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/plot.cpp
)
target_link_libraries(emrainey-linalg-utils PUBLIC
    Linalg::emrainey-linalg Geometry::emrainey-geometry Fourcc::emrainey-fourcc ${OpenCV_LIBS})
target_include_directories(emrainey-linalg-utils
    PUBLIC
        # Generated Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        # Build Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # Install Tree
        $<INSTALL_INTERFACE:include>
        ${OpenCV_INCLUDE_DIRS}
)
endif()

#####################
# EXPORTS
#####################

# Declare installs and exports (this isn't working yet)
install(TARGETS emrainey-linalg-utils EXPORT LinalgUtilsTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(EXPORT LinalgUtilsTargets
    FILE LinalgUtilsTargets.cmake
    NAMESPACE LinalgUtils::
    DESTINATION share/cmake/linalg-utils
)
install(DIRECTORY include/linalg
    DESTINATION include
    PATTERN "*.hpp"
    PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
)
install(FILES "cmake/Findlinalg-utils.cmake" "cmake/linalg-utils-config.cmake"
    DESTINATION share/cmake/linalg-utils
)
#####################
# Demos
#####################

if (${OpenCV_FOUND})
add_executable(demo_least_squares ${CMAKE_CURRENT_SOURCE_DIR}/demo/demo_least_squares.cpp)
target_include_directories(demo_least_squares PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(demo_least_squares emrainey-linalg-utils)

add_executable(demo_images ${CMAKE_CURRENT_SOURCE_DIR}/demo/demo_images.cpp)
target_include_directories(demo_images PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(demo_images emrainey-linalg-utils)
endif()

#####################
# Build Documentation
#####################

if (${Doxygen_FOUND})
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(emrainey-linalg-utils-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/linalg
        COMMENT "Linalg Doxygen")
endif()