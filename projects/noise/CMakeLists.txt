# === Project ===
cmake_minimum_required(VERSION 3.31.0)
project(noise LANGUAGES CXX VERSION 0.7.0)  # This must match the conanfile.py!

# === Dependencies ===
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(OpenCV)
find_package(Threads)
find_package(GTest)
find_package(Doxygen)

# === Targets ===
add_library(hobbies-noise
    ${CMAKE_CURRENT_SOURCE_DIR}/source/noise.cpp
)
target_link_libraries(hobbies-noise
    PUBLIC
        hobbies-fourcc
        hobbies-geometry
        $<$<BOOL:${USE_XMMT}>:hobbies-xmmt>
    PRIVATE
        enabled-warnings
        enabled-debugging
        $<$<BOOL:${USE_PROFILING}>:enabled-profiling>
        $<$<BOOL:${USE_COVERAGE}>:enabled-coverage>
        native-optimized
)
target_include_directories(hobbies-noise
    PUBLIC
        # Generated Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        # Build Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # Install Tree
        $<INSTALL_INTERFACE:include>
)
target_compile_definitions(hobbies-noise PUBLIC
    $<$<BOOL:${USE_XMMT}>:USE_XMMT>
)
set_target_properties(hobbies-noise PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

# === Demos ===
if (OpenCV_FOUND)
add_executable(demo_turbulent_tuner ${CMAKE_CURRENT_SOURCE_DIR}/demo/main_turbulent_tuner.cpp)
target_link_libraries(demo_turbulent_tuner ${OpenCV_LIBS} hobbies-noise hobbies-linalg-utils)

add_executable(demo_perlin_tuner ${CMAKE_CURRENT_SOURCE_DIR}/demo/perlin_tuner.cpp)
target_link_libraries(demo_perlin_tuner ${OpenCV_LIBS} hobbies-noise hobbies-linalg-utils)

add_executable(demo_fbm_tuner ${CMAKE_CURRENT_SOURCE_DIR}/demo/fbm_tuner.cpp)
target_link_libraries(demo_fbm_tuner ${OpenCV_LIBS} hobbies-noise hobbies-linalg-utils)
endif(OpenCV_FOUND)

# === Googletests ===
if (BUILD_UNIT_TESTS AND Threads_FOUND AND GTest_FOUND)
    add_executable(gtest_noise
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_noise.cpp
    )
    target_link_libraries(gtest_noise PRIVATE hobbies-noise hobbies-fourcc enabled-debugging GTest::gtest GTest::gmock GTest::gtest_main Threads::Threads)
    add_test(NAME gtest_noise COMMAND gtest_noise)
endif()

# === Doxygen ===
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-noise-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/noise
        COMMENT "Noise Doxygen")
endif()