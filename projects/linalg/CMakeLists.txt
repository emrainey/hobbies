# === Project ===
cmake_minimum_required(VERSION 3.31.0)
project(linalg LANGUAGES CXX VERSION 0.7.0)  # This must match the conanfile.py!

# === Dependencies ===
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
find_package(GTest)
find_package(Doxygen)

# === Targets ===
add_library(hobbies-linalg
    ${CMAKE_CURRENT_SOURCE_DIR}/source/matrix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/solvers.cpp
)
target_link_libraries(hobbies-linalg
    PUBLIC
        hobbies-basal
        # hobbies-geometry
        hobbies-uom
    PRIVATE
        enabled-warnings
        enabled-debugging
        $<$<BOOL:${USE_PROFILING}>:enabled-profiling>
        $<$<BOOL:${USE_COVERAGE}>:enabled-coverage>
        native-optimized
)
target_include_directories(hobbies-linalg PUBLIC
    # Generated Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    # Build Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    # Install Tree
    $<INSTALL_INTERFACE:include>
)
set_target_properties(hobbies-linalg PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)
install(TARGETS hobbies-linalg
    EXPORT linalg-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/linalg
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# === Googletests ===
if (BUILD_UNIT_TESTS AND Threads_FOUND AND GTest_FOUND)
    add_executable(gtest_linalg
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_solvers.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_matrix.cpp
    )
    target_link_libraries(gtest_linalg PRIVATE hobbies-linalg enabled-debugging GTest::gtest GTest::gtest_main Threads::Threads)
    add_test(NAME gtest_linalg COMMAND gtest_linalg)
endif()

# === Doxygen ===
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-linalg-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/linalg
        COMMENT "Linalg Doxygen")
endif()