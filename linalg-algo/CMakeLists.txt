cmake_minimum_required(VERSION 3.20.0)
project(linalg-algo LANGUAGES CXX VERSION 0.7.0)  # This must match the conanfile.py!

find_package(OpenCV REQUIRED)
find_package(basal 0.5.0 REQUIRED)
find_package(units_of_measure REQUIRED)
find_package(fourcc REQUIRED)
find_package(linalg REQUIRED)
find_package(linalg-utils REQUIRED)

# Build Everything with C++17 and full native optimizations
set(CMAKE_CXX_STANDARD 17)
enable_testing()

option(BUILD_SHARED_LIBS "Builds libraries as dynamic objects" OFF)

if (OpenCV_FOUND)
add_library(hobbies-linalg-algo
    ${CMAKE_CURRENT_SOURCE_DIR}/source/kmeans.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/dkf.cpp
)
target_link_libraries(hobbies-linalg-algo PUBLIC
    Geometry::hobbies-geometry
    LinalgUtils::hobbies-linalg-utils
)
target_include_directories(hobbies-linalg-algo
    PUBLIC
        # Generated Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        # Build Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # Install Tree
        $<INSTALL_INTERFACE:include>
)
set_target_properties(hobbies-linalg-algo PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
#####################
# EXPORTS
#####################

# Declare installs and exports
install(TARGETS hobbies-linalg-algo EXPORT LinalgAlgoTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(EXPORT LinalgAlgoTargets
    FILE LinalgAlgoTargets.cmake
    NAMESPACE LinalgAlgo::
    DESTINATION share/cmake/${PROJECT_NAME}
)
install(DIRECTORY include/linalg
    DESTINATION include
    PATTERN "*.hpp"
    PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
)
install(FILES "cmake/Find${PROJECT_NAME}.cmake" "cmake/${PROJECT_NAME}-config.cmake"
    DESTINATION share/cmake/${PROJECT_NAME}
)

#####################
# Demos
#####################

add_executable(demo_kmeans ${CMAKE_CURRENT_SOURCE_DIR}/demo/demo_kmeans.cpp)
target_include_directories(demo_kmeans PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(demo_kmeans hobbies-linalg-algo)

#####################
# Build Documentation
#####################

find_package(Doxygen)
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-linalg-algo-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/linalg
        COMMENT "Linalg + Algo Doxygen")
endif()

endif() # OpenCV
