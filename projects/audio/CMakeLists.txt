# === Project ===
cmake_minimum_required(VERSION 3.31.0)
project(audio LANGUAGES CXX VERSION 0.7.0)  # This must match the conanfile.py!

# === Dependencies ===
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
find_package(GTest)
find_package(Doxygen)

# === Targets ===
add_library(hobbies-audio
    ${CMAKE_CURRENT_SOURCE_DIR}/source/audio.cpp
)
target_include_directories(hobbies-audio
    PUBLIC
        # Generated Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        # Build Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # Install Tree
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(hobbies-audio
    PUBLIC
        hobbies-basal
        hobbies-uom
    PRIVATE
        enabled-warnings
        enabled-debugging
        $<$<BOOL:${USE_PROFILING}>:enabled-profiling>
        $<$<BOOL:${USE_COVERAGE}>:enabled-coverage>
        native-optimized
)
set_target_properties(hobbies-audio PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)
install(TARGETS hobbies-audio
    EXPORT audio-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/audio
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# Visualization of audio with the FFT, used by the demos
add_library(hobbies-fft-vis
    ${CMAKE_CURRENT_SOURCE_DIR}/source/fft_vis.cpp
)
target_include_directories(hobbies-fft-vis
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(hobbies-fft-vis
    PUBLIC
        hobbies-audio
        hobbies-fourcc
    PRIVATE
        enabled-warnings
        enabled-debugging
        native-optimized
)
set_target_properties(hobbies-fft-vis PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)
install(TARGETS hobbies-fft-vis
    EXPORT audio-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/fft_vis
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# === Googletests ===
if (BUILD_UNIT_TESTS AND Threads_FOUND AND GTest_FOUND)
    add_executable(gtest_audio
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_audio.cpp
    )
    target_link_libraries(gtest_audio PRIVATE hobbies-audio enabled-debugging GTest::gtest GTest::gmock GTest::gtest_main Threads::Threads)
    add_test(NAME gtest_audio COMMAND gtest_audio)

    add_executable(gtest_fft_vis
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_fft_vis.cpp
    )
    target_link_libraries(gtest_fft_vis PRIVATE hobbies-fft-vis enabled-debugging GTest::gtest GTest::gmock GTest::gtest_main Threads::Threads)
    add_test(NAME gtest_fft_vis COMMAND gtest_fft_vis)
endif()

# === Demos ===
add_executable(demo_noise_effects ${CMAKE_CURRENT_SOURCE_DIR}/demo/main_noise_effects.cpp)
target_link_libraries(demo_noise_effects PRIVATE hobbies-audio hobbies-fft-vis hobbies-noise enabled-warnings)
add_executable(demo_fft_vis ${CMAKE_CURRENT_SOURCE_DIR}/demo/main_fft_vis.cpp)
target_link_libraries(demo_fft_vis PRIVATE hobbies-fft-vis enabled-warnings)
install(TARGETS demo_noise_effects demo_fft_vis RUNTIME DESTINATION bin)

# === Doxygen ===
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-audio-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/audio
        COMMENT "audio Doxygen")
endif()