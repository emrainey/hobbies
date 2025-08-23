# === Project ===
cmake_minimum_required(VERSION 3.31.0)
project(basal LANGUAGES CXX VERSION 0.5.0)  # This must match the conanfile.py!

# === Dependencies ===
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
find_package(GTest)
find_package(Doxygen)

# === Options ===
option(USE_PRECISION_AS_FLOAT "Use float (ON) as the precision type or double (OFF)" OFF)

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
        $<$<BOOL:${USE_PROFILING}>:enabled-profiling>
        $<$<BOOL:${USE_COVERAGE}>:enabled-coverage>
        native-optimized
)
set_target_properties(hobbies-basal PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

# === Unit Tests ===
if (Threads_FOUND AND GTest_FOUND)
    add_executable(gtest_basal
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_basal.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_fraction.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_listable.cpp
    )
    target_link_libraries(gtest_basal PRIVATE hobbies-basal GTest::gtest GTest::gtest_main Threads::Threads)
    add_test(NAME gtest_basal COMMAND gtest_basal)
endif()

# === Doxygen ===
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-basal-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        COMMENT "Basal Doxygen")
endif()
