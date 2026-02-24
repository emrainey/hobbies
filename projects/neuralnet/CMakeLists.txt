# === Project ===
cmake_minimum_required(VERSION 3.31.0)
project(neuralnet LANGUAGES CXX VERSION 0.7.0)  # This must match the conanfile.py!

# === Dependencies ===
find_package(OpenCV)
find_package(Threads)
find_package(GTest)
find_package(Doxygen)
find_package(OpenMP)

# === Targets ===
if (OpenCV_FOUND)
add_library(hobbies-neuralnet
    ${CMAKE_CURRENT_SOURCE_DIR}/source/hidden.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/inner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/layer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/matrix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mnist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/network.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/output.cpp
)
target_link_libraries(hobbies-neuralnet
    PUBLIC
        hobbies-basal
        hobbies-linalg
        hobbies-linalg-utils
        ${OpenCV_LIBS}
        $<$<BOOL:${OpenMP_CXX_FOUND}>:OpenMP::OpenMP_CXX>
    PRIVATE
        enabled-warnings
        enabled-debugging
        $<$<BOOL:${USE_PROFILING}>:enabled-profiling>
        $<$<BOOL:${USE_COVERAGE}>:enabled-coverage>
        native-optimized
)
target_include_directories(hobbies-neuralnet
    PUBLIC
       # Generated Tree
       $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
       # Build Tree
       $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
       # Install Tree
       $<INSTALL_INTERFACE:include>
)
set_target_properties(hobbies-neuralnet PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)
install(TARGETS hobbies-neuralnet
    EXPORT neuralnet-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/neuralnet
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# === Demos ===
add_executable(demo_neuralnet ${CMAKE_CURRENT_SOURCE_DIR}/demo/neuralnet_counter.cpp)
target_link_libraries(demo_neuralnet hobbies-neuralnet ${OpenCV_LIBS})

add_executable(neuralnet ${CMAKE_CURRENT_SOURCE_DIR}/source/neuralnet.cpp)
target_link_libraries(neuralnet hobbies-neuralnet ${OpenCV_LIBS})

install(TARGETS neuralnet demo_neuralnet
    EXPORT neuralnet-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# === Googletests ===
if (BUILD_UNIT_TESTS AND Threads_FOUND AND GTest_FOUND)
    add_executable(gtest_neuralnet
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/gtest_neuralnet.cpp
    )
    target_link_libraries(gtest_neuralnet PRIVATE hobbies-neuralnet enabled-debugging GTest::gtest GTest::gmock GTest::gtest_main Threads::Threads)
    add_test(NAME gtest_neuralnet COMMAND gtest_neuralnet)
endif()

# === Doxygen ===
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-neuralnet-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/neuralnet
        COMMENT "Neural Network Doxygen")
endif()

endif(OpenCV_FOUND)
