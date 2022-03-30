cmake_minimum_required(VERSION 3.20.0)
project(noise CXX)

find_package(OpenCV REQUIRED)
find_package(basal REQUIRED)
find_package(units_of_measure REQUIRED)
find_package(fourcc REQUIRED)
find_package(linalg REQUIRED)
find_package(geometry REQUIRED)
find_package(linalg-utils REQUIRED)
find_package(xmmt REQUIRED)

# Build Everything with C++17 and full native optimizations
set(CMAKE_CXX_STANDARD 17)
enable_testing()

option(BUILD_SHARED_LIBS "Builds libraries as dynamic objects" OFF)

add_library(hobbies-noise
    ${CMAKE_CURRENT_SOURCE_DIR}/source/noise.cpp
)
target_link_libraries(hobbies-noise PUBLIC
    Fourcc::hobbies-fourcc Geometry::hobbies-geometry Xmmt::hobbies-xmmt
)
target_include_directories(hobbies-noise
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
install(TARGETS hobbies-noise EXPORT NoiseTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(EXPORT NoiseTargets
    FILE NoiseTargets.cmake
    NAMESPACE Noise::
    DESTINATION share/cmake/noise
)
install(DIRECTORY include/noise
    DESTINATION include
    PATTERN "*.hpp"
    PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
)
install(FILES "cmake/Findnoise.cmake" "cmake/noise-config.cmake"
    DESTINATION share/cmake/noise
)
################
# DEMOS
################

add_executable(demo_turbulent_tuner ${CMAKE_CURRENT_SOURCE_DIR}/demo/main_turbulent_tuner.cpp)
target_link_libraries(demo_turbulent_tuner ${OpenCV_LIBS} hobbies-noise LinalgUtils::hobbies-linalg-utils)

add_executable(demo_perlin_tuner ${CMAKE_CURRENT_SOURCE_DIR}/demo/perlin_tuner.cpp)
target_link_libraries(demo_perlin_tuner ${OpenCV_LIBS} hobbies-noise LinalgUtils::hobbies-linalg-utils)

#####################
# Googletests
#####################
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
if (GTest_FOUND)
    add_executable(gtest_noise
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_noise.cpp
    )
    target_include_directories(gtest_noise PUBLIC ${GTEST_INCLUDE_DIRS})
    target_link_libraries(gtest_noise PRIVATE ${GTEST_BOTH_LIBRARIES} hobbies-noise Fourcc::hobbies-fourcc Threads::Threads)
    add_test(NAME gtest_noise COMMAND gtest_noise)
endif()

#####################
# Build Documentation
#####################
find_package(Doxygen)
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-noise-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/noise
        COMMENT "Noise Doxygen")
endif()