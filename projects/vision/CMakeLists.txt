# === Project ===
cmake_minimum_required(VERSION 3.31.0)
project(vision LANGUAGES CXX VERSION 0.1.0)  # This must match the conanfile.py!

# === Dependencies ===
# set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(OpenCV)
# find_package(Threads)
# find_package(GTest)
# find_package(Doxygen)

# === Targets ===
# add_library(hobbies-vision
#     ${CMAKE_CURRENT_SOURCE_DIR}/source/vision.cpp
# )
# target_link_libraries(hobbies-vision
#     PUBLIC
#         hobbies-fourcc
#         hobbies-geometry
#         $<$<BOOL:${USE_XMMT}>:hobbies-xmmt>
#     PRIVATE
#         enabled-warnings
#         enabled-debugging
#         $<$<BOOL:${USE_PROFILING}>:enabled-profiling>
#         native-optimized
# )
# target_include_directories(hobbies-vision
#     PUBLIC
#         # Generated Tree
#         $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
#         # Build Tree
#         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#         # Install Tree
#         $<INSTALL_INTERFACE:include>
# )
# if (USE_XMMT)
#     target_compile_definitions(hobbies-vision PUBLIC USE_XMMT)
# endif()
# set_target_properties(hobbies-vision PROPERTIES
#     VERSION ${PROJECT_VERSION}
#     SOVERSION ${PROJECT_VERSION_MAJOR}
#     POSITION_INDEPENDENT_CODE ON
# )

# === Demos ===
if (OpenCV_FOUND)
add_executable(demo_artsy ${CMAKE_CURRENT_SOURCE_DIR}/demo/main_artsy.cpp)
target_link_libraries(demo_artsy
    PUBLIC
        ${OpenCV_LIBS}
        hobbies-basal
    PRIVATE
        enabled-warnings
        enabled-debugging
        $<$<BOOL:${USE_PROFILING}>:enabled-profiling>
        native-optimized
)

add_executable(demo_evm ${CMAKE_CURRENT_SOURCE_DIR}/demo/main_evm.cpp)
target_link_libraries(demo_evm
    PUBLIC
        ${OpenCV_LIBS}
        hobbies-basal
        hobbies-uom
    PRIVATE
        enabled-warnings
        enabled-debugging
        $<$<BOOL:${USE_PROFILING}>:enabled-profiling>
        native-optimized
)

# === Googletests ===
# if (BUILD_UNIT_TESTS AND Threads_FOUND AND GTest_FOUND)
#     add_executable(gtest_vision
#         ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_vision.cpp
#     )
#     target_link_libraries(gtest_vision PRIVATE hobbies-vision hobbies-fourcc enabled-debugging GTest::gtest GTest::gmock GTest::gtest_main Threads::Threads)
#     add_test(NAME gtest_vision COMMAND gtest_vision)
# endif()

# === Doxygen ===
# if (Doxygen_FOUND)
#     set(DOXYGEN_GENERATE_HTML YES)
#     set(DOXYGEN_GENERATE_MAN NO)
#     doxygen_add_docs(hobbies-vision-docs
#         ${CMAKE_CURRENT_SOURCE_DIR}/include/vision
#         COMMENT "Noise Doxygen")
# endif()

endif(OpenCV_FOUND)