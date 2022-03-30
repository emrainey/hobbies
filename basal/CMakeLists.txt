cmake_minimum_required(VERSION 3.20.0)
project(basal CXX)
set(CMAKE_CXX_STANDARD 17)
enable_testing()

option(BUILD_SHARED_LIBS "Builds libraries as dynamic objects" OFF)

add_library(native-optimized INTERFACE)
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU|Clang")
target_compile_options(native-optimized INTERFACE -ggdb -march=native -mtune=native)
# add -pg above (might have problems in shared object environment w/ multiple symbols)
# target_link_options(native-optimized INTERFACE -pg)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "x64_64")
target_compile_options(native-optimized INTERFACE /arch:AVX2)
endif()

add_library(hobbies-basal)
target_sources(hobbies-basal
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source/debuggable.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/exception.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/listable.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/options.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/module.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/basal.cpp
)
target_include_directories(hobbies-basal PUBLIC
    # Generated Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    # Build Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    # Install Tree
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(hobbies-basal dl)

# Declare installs and exports
install(TARGETS hobbies-basal native-optimized EXPORT BasalTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(EXPORT BasalTargets
    FILE BasalTargets.cmake
    NAMESPACE Basal::
    DESTINATION share/cmake/basal
    PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
)
install(DIRECTORY include/basal
    DESTINATION include
    PATTERN "*.hpp"
    PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
)
install(FILES "cmake/Findbasal.cmake" "cmake/basal-config.cmake"
    DESTINATION share/cmake/basal
    PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
)
#include(CMakePackageConfigHelpers)
#write_basic_package_version_file("BasalConfigVersion.cmake"
#    VERSION ${Basal_VERSION}
#    COMPATIBILITY SameMajorVersion
#)
#install(FILES "cmake/Findbasal.cmake" "cmake/basal-config.cmake" "basalConfigVersion.cmake"
#    DESTINATION share/cmake/basal
#)

# Unit Tests
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
if (GTest_FOUND)
    message(STATUS "GTest_FOUND=${GTest_FOUND}")
    message(STATUS "GTEST_INCLUDE_DIRS=${GTEST_INCLUDE_DIRS}")
    message(STATUS "GTEST_BOTH_LIBRARIES=${GTEST_BOTH_LIBRARIES}")
    add_executable(gtest_basal
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_basal.cpp
    )
    target_include_directories(gtest_basal PRIVATE ${GTEST_INCLUDE_DIRS} ${GTest_INCLUDE_DIRS})
    target_link_libraries(gtest_basal PRIVATE hobbies-basal ${GTEST_BOTH_LIBRARIES} ${GTest_LIBS} Threads::Threads)
    add_test(NAME gtest_basal COMMAND gtest_basal)
endif()

# Documentation
find_package(Doxygen REQUIRED)
if (${Doxygen_FOUND})
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-basal-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        COMMENT "Basal Doxygen")
endif()