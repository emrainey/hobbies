cmake_minimum_required(VERSION 3.20.0)
project(basal LANGUAGES CXX VERSION 0.5.0)  # This must match the conanfile.py!
set(CMAKE_CXX_STANDARD 17)
enable_testing()

option(BUILD_SHARED_LIBS "Builds libraries as dynamic objects" OFF)

add_library(enabled-warnings INTERFACE)
target_compile_options(enabled-warnings INTERFACE
    $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang,GNU>:$<BUILD_INTERFACE:-Wall -Wextra -Werror -pedantic>>
)

# Optimizes your objects for native builds (on the build side only)
add_library(native-optimized INTERFACE)
target_compile_options(native-optimized INTERFACE
    $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang,GNU>:-ggdb -march=native -mtune=native>
    $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/arch:AVX2>
)


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
target_link_libraries(hobbies-basal dl enabled-warnings)
set_target_properties(hobbies-basal PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Declare installs and exports
install(TARGETS hobbies-basal enabled-warnings native-optimized
    EXPORT BasalTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(EXPORT BasalTargets
    FILE BasalTargets.cmake
    NAMESPACE Basal::
    DESTINATION share/cmake/${PROJECT_NAME}
    PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
)
install(DIRECTORY include/${PROJECT_NAME}
    DESTINATION include
    PATTERN "*.hpp"
    PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
)
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${PROJECT_NAME}-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
        "cmake/Find${PROJECT_NAME}.cmake"
        "cmake/${PROJECT_NAME}-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    DESTINATION share/cmake/${PROJECT_NAME}
    PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
)

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