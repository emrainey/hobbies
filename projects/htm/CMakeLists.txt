# === Project ===
cmake_minimum_required(VERSION 3.31.0)
project(htm LANGUAGES CXX VERSION 0.1.0)  # This must match the conanfile.py!

# === Dependencies ===
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
find_package(GTest)
find_package(Doxygen)

# === Targets ===
add_library(hobbies-htm
    ${CMAKE_CURRENT_SOURCE_DIR}/source/encoders.cpp
)
target_link_libraries(hobbies-htm
    PUBLIC
        hobbies-basal
    PRIVATE
        enabled-debugging
        $<$<BOOL:${USE_PROFILING}>:enabled-profiling>
        enabled-warnings
        $<$<BOOL:${USE_COVERAGE}>:enabled-coverage>
        native-optimized
)
target_include_directories(hobbies-htm
    PUBLIC
        # Generated Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        # Build Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # Install Tree
        $<INSTALL_INTERFACE:include>
)
set_target_properties(hobbies-htm PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

# === Demos ===
add_executable(demo_sparse_word ${CMAKE_CURRENT_SOURCE_DIR}/demo/sparse_word.cpp)
target_link_libraries(demo_sparse_word hobbies-htm)

# === Unit Tests ===
if (Threads_FOUND AND GTest_FOUND)
    add_executable(gtest_htm
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_words.cpp
    )
    target_link_libraries(gtest_htm PRIVATE hobbies-htm GTest::gtest GTest::gtest_main Threads::Threads)
    add_test(NAME gtest_htm COMMAND gtest_htm)
endif()

# === Doxygen ===
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-htm-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/htm
        COMMENT "HTM Doxygen")
endif()