cmake_minimum_required(VERSION 3.20.0)
project(basal LANGUAGES CXX VERSION 0.5.0)  # This must match the conanfile.py!
set(CMAKE_CXX_STANDARD 17)
enable_testing()

option(BUILD_SHARED_LIBS "Builds libraries as dynamic objects" OFF)
option(USE_PRECISION_AS_FLOAT "Use float (ON) as the precision type or double (OFF)" OFF)

add_library(enabled-warnings INTERFACE)
target_compile_options(enabled-warnings INTERFACE
    $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang,GNU>:$<BUILD_INTERFACE:-Wall -Wextra -Werror -pedantic>>
)

add_library(enabled-debugging INTERFACE)
target_compile_options(enabled-debugging INTERFACE
    $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:-glldb>
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-g -gstabs+>
)

add_library(enabled-profiling INTERFACE)
target_link_options(enabled-profiling INTERFACE
    $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:-fprofile-instr-generate>
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-pg>
)
target_compile_options(enabled-profiling INTERFACE
    $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:-fprofile-instr-generate>
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-pg>
)

# Optimizes your objects for native builds (on the build side only)
add_library(native-optimized INTERFACE)
target_compile_options(native-optimized INTERFACE
    $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang,GNU>:-march=native -mtune=native>
    $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/arch:AVX2>
)


add_library(hobbies-basal)
target_sources(hobbies-basal
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source/debuggable.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/exception.cpp
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
target_compile_definitions(hobbies-basal
    PUBLIC
        $<$<BOOL:${USE_PRECISION_AS_FLOAT}>:USE_PRECISION_AS_FLOAT>
)
target_link_libraries(hobbies-basal
    PUBLIC
        dl
    PRIVATE
        enabled-warnings
        enabled-debugging
        enabled-profiling
        native-optimized
)
set_target_properties(hobbies-basal PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

# Declare installs and exports
install(TARGETS hobbies-basal enabled-warnings enabled-debugging enabled-profiling native-optimized
    EXPORT BasalTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(EXPORT BasalTargets
    FILE ${PROJECT_NAME}-Targets.cmake
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
        # "cmake/Find${PROJECT_NAME}.cmake"
        "cmake/${PROJECT_NAME}-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    DESTINATION share/cmake/${PROJECT_NAME}
    PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
)

# Unit Tests
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
find_package(GTest)
if (Threads_FOUND AND GTest_FOUND)
    add_executable(gtest_basal
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_basal.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_listable.cpp
    )
    target_link_libraries(gtest_basal PRIVATE hobbies-basal GTest::gtest GTest::gtest_main Threads::Threads)
    add_test(NAME gtest_basal COMMAND gtest_basal)
endif()

# Documentation
find_package(Doxygen)
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-basal-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        COMMENT "Basal Doxygen")
endif()
