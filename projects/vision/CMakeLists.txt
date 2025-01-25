cmake_minimum_required(VERSION 3.20.0)
project(vision LANGUAGES CXX VERSION 0.1.0)  # This must match the conanfile.py!

find_package(OpenCV REQUIRED)
option(USE_XMMT "Use XMMT for SIMD" ON)

# Use XMMT if we're building for x86_64
if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
set(USE_XMMT ON)
message(STATUS "Enabling XMMT")
else()
set(USE_XMMT OFF)
message(WARNING "Disabling XMMT")
endif()

# add_library(hobbies-vision
#     ${CMAKE_CURRENT_SOURCE_DIR}/source/vision.cpp
# )
# target_link_libraries(hobbies-vision
#     PUBLIC
#         hobbies-fourcc
#         hobbies-geometry
#         hobbies-xmmt
#     PRIVATE
#         enabled-warnings
#         enabled-debugging
#         enabled-profiling
#         native-optimized
# )
# target_include_directories(hobbies-vision
#     PUBLIC
#         # Generated Tree
#         $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
#         # Build Tree
#         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#         # Install Tree
#         $<INSTALL_INTERFACE:include>
# )
# if (USE_XMMT)
#     target_compile_definitions(hobbies-vision PUBLIC USE_XMMT)
# endif()
# set_target_properties(hobbies-vision PROPERTIES
#     VERSION ${PROJECT_VERSION}
#     SOVERSION ${PROJECT_VERSION_MAJOR}
#     POSITION_INDEPENDENT_CODE ON
# )

#####################
# EXPORTS
#####################

# Declare installs and exports
# install(TARGETS hobbies-vision EXPORT NoiseTargets
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
#     RUNTIME DESTINATION bin
#     INCLUDES DESTINATION include
# )
# install(EXPORT NoiseTargets
#     FILE ${PROJECT_NAME}-Targets.cmake
#     NAMESPACE Noise::
#     DESTINATION share/cmake/${PROJECT_NAME}
# )
# install(DIRECTORY include/${PROJECT_NAME}
#     DESTINATION include
#     PATTERN "*.hpp"
#     PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
# )
# install(FILES "cmake/Find${PROJECT_NAME}.cmake" "cmake/${PROJECT_NAME}-config.cmake"
#     DESTINATION share/cmake/${PROJECT_NAME}
# )
################
# DEMOS
################

add_executable(demo_artsy ${CMAKE_CURRENT_SOURCE_DIR}/demo/main_artsy.cpp)
target_link_libraries(demo_artsy ${OpenCV_LIBS})

#####################
# Googletests
#####################
# set(THREADS_PREFER_PTHREAD_FLAG ON)
# find_package(Threads)
# find_package(GTest)
# if (Threads_FOUND AND GTest_FOUND)
#     add_executable(gtest_vision
#         ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_vision.cpp
#     )
#     target_link_libraries(gtest_vision PRIVATE hobbies-vision hobbies-fourcc enabled-debugging GTest::gtest GTest::gtest_main Threads::Threads)
#     add_test(NAME gtest_vision COMMAND gtest_vision)
# endif()

#####################
# Build Documentation
#####################
# find_package(Doxygen)
# if (Doxygen_FOUND)
#     set(DOXYGEN_GENERATE_HTML YES)
#     set(DOXYGEN_GENERATE_MAN NO)
#     doxygen_add_docs(hobbies-vision-docs
#         ${CMAKE_CURRENT_SOURCE_DIR}/include/vision
#         COMMENT "Noise Doxygen")
# endif()