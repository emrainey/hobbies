# === Project ===
cmake_minimum_required(VERSION 3.31.0)
project(linalg-algo LANGUAGES CXX VERSION 0.7.0)  # This must match the conanfile.py!

# === Dependencies ===
find_package(OpenCV)
find_package(GTest)

# === Targets ===
add_library(hobbies-linalg-algo
    ${CMAKE_CURRENT_SOURCE_DIR}/source/kmeans.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/dkf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/softmax.cpp
)
target_link_libraries(hobbies-linalg-algo PUBLIC
    hobbies-geometry
    hobbies-linalg-utils
)
target_include_directories(hobbies-linalg-algo
    PUBLIC
        # Generated Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        # Build Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # Install Tree
        $<INSTALL_INTERFACE:include>
)
set_target_properties(hobbies-linalg-algo PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)
install(TARGETS hobbies-linalg-algo
    EXPORT linalg-algo-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/linalg-algo
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# === Demos ===
if (OpenCV_FOUND)
add_executable(demo_kmeans ${CMAKE_CURRENT_SOURCE_DIR}/demo/demo_kmeans.cpp)
target_link_libraries(demo_kmeans hobbies-linalg-algo ${OpenCV_LIBS})
endif()

# === Googletests ===
if (BUILD_UNIT_TESTS AND GTest_FOUND)
    add_executable(gtest_softmax
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_softmax.cpp
    )
    target_link_libraries(gtest_softmax PRIVATE hobbies-linalg-algo GTest::gtest GTest::gmock GTest::gtest_main)
    add_test(NAME gtest_softmax COMMAND gtest_softmax)
endif()

# === Google Benchmarks ===
find_package(benchmark)
if (BUILD_UNIT_TESTS AND benchmark_FOUND)
    add_executable(gbench_algo
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gbench_algo.cpp
    )
    target_link_libraries(gbench_algo PRIVATE hobbies-linalg-algo benchmark::benchmark)
    add_test(NAME gbench_algo COMMAND gbench_algo)
endif()

# === Doxygen ===
find_package(Doxygen)
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-linalg-algo-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/linalg
        COMMENT "Linalg + Algo Doxygen")
endif()
