# === Project ===
cmake_minimum_required(VERSION 3.31.0)
project(xmmt LANGUAGES CXX VERSION 0.7.0)  # This must match the conanfile.py!

# === Dependencies ===
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
find_package(GTest)
find_package(Doxygen)

# === Options ===

# === Targets ===
add_library(hobbies-xmmt INTERFACE)
target_include_directories(hobbies-xmmt INTERFACE
    # Generated Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    # Build Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    # Install Tree
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(hobbies-xmmt INTERFACE hobbies-basal native-optimized)

# === Googletests ===
if (USE_XMMT AND BUILD_UNIT_TESTS AND Threads_FOUND AND GTest_FOUND)
    add_executable(gtest_xmmt
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_xmmt.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_xmat.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_pixel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_point.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_vector.cpp
    )
    target_link_libraries(gtest_xmmt PRIVATE hobbies-xmmt enabled-debugging GTest::gtest GTest::gmock GTest::gtest_main Threads::Threads)
    add_test(NAME gtest_xmmt COMMAND gtest_xmmt)
endif()

# === Doxygen ===
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-xmmt-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/xmmt
        COMMENT "XMMT Doxygen")
endif()