cmake_minimum_required(VERSION 3.20.0)
project(basal
    VERSION 0.7
    LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
enable_testing()

add_library(emrainey-basal SHARED)
target_sources(emrainey-basal
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source/debuggable.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/exception.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/listable.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/basal.cpp
)
target_include_directories(emrainey-basal PUBLIC
    # Generated Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    # Build Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    # Install Tree
    $<INSTALL_INTERFACE:include>
)

# Declare installs and exports
install(TARGETS emrainey-basal EXPORT BasalTargets
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
install(FILES "cmake/Findbasal.cmake" "cmake/basalConfig.cmake"
    DESTINATION share/cmake/basal
    PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
)
#include(CMakePackageConfigHelpers)
#write_basic_package_version_file("BasalConfigVersion.cmake"
#    VERSION ${Basal_VERSION}
#    COMPATIBILITY SameMajorVersion
#)
#install(FILES "cmake/Findbasal.cmake" "cmake/basalConfig.cmake" "basalConfigVersion.cmake"
#    DESTINATION share/cmake/basal
#)

# Unit Tests
find_package(GTest REQUIRED)
if (${GTest_FOUND})
    add_executable(gtest_basal
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_basal.cpp
    )
    target_include_directories(gtest_basal PRIVATE ${GTEST_INCLUDE_DIRS})
    target_link_libraries(gtest_basal emrainey-basal ${GTEST_BOTH_LIBRARIES})
    add_test(NAME gtest_basal COMMAND gtest_basal)
endif()

# Documentation
find_package(Doxygen REQUIRED)
if (${Doxygen_FOUND})
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(emrainey-basal-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        COMMENT "Basal Doxygen")
endif()