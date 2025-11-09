# === Project ===
cmake_minimum_required(VERSION 3.31.0)
project(fourcc LANGUAGES CXX VERSION 0.7.0)  # This must match the conanfile.py!

# === Dependencies ===
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
find_package(GTest)
find_package(Doxygen)

# === Targets ===
add_library(hobbies-fourcc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/convolve.cpp
)
target_include_directories(hobbies-fourcc
    PUBLIC
        # Generated Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        # Build Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # Install Tree
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(hobbies-fourcc
    PUBLIC
        hobbies-basal
    PRIVATE
        enabled-warnings
        enabled-debugging
        $<$<BOOL:${USE_PROFILING}>:enabled-profiling>
        $<$<BOOL:${USE_COVERAGE}>:enabled-coverage>
        native-optimized
)
set_target_properties(hobbies-fourcc PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

# === Googletests ===
if (BUILD_UNIT_TESTS AND Threads_FOUND AND GTest_FOUND)
    add_executable(gtest_fourcc
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_fourcc.cpp
    )
    target_link_libraries(gtest_fourcc PRIVATE hobbies-fourcc enabled-debugging GTest::gtest GTest::gmock GTest::gtest_main Threads::Threads)
    add_test(NAME gtest_fourcc COMMAND gtest_fourcc)
endif()

# === Doxygen ===
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-fourcc-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/fourcc
        COMMENT "Fourcc Doxygen")
endif()