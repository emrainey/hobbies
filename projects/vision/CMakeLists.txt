# === Project ===
cmake_minimum_required(VERSION 3.31.0)
project(vision LANGUAGES CXX VERSION 0.1.0)  # This must match the conanfile.py!

# Apple Vision subject segmentation (source/subject_mask.mm) is Objective-C++ and links
# the system Vision/CoreGraphics/Foundation frameworks on macOS 14+.
if (APPLE AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    enable_language(OBJCXX)
    find_library(VISION_FRAMEWORK Vision REQUIRED)
    find_library(COREGRAPHICS_FRAMEWORK CoreGraphics REQUIRED)
    find_library(COREVIDEO_FRAMEWORK CoreVideo REQUIRED)
    find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
endif()

# === Dependencies ===
# set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(OpenCV)
find_package(Threads)
find_package(GTest)
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

add_executable(chroma-replace
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/main_chroma_replace.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/chroma_replace.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/frame_spec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/matting.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/protect_spec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/subject_mask.mm
)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/source/subject_mask.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc;-Wno-error;-Wno-deprecated-declarations")
target_include_directories(chroma-replace
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(chroma-replace
    PUBLIC
        ${OpenCV_LIBS}
        hobbies-basal
    PRIVATE
        enabled-warnings
        enabled-debugging
        $<$<BOOL:${USE_PROFILING}>:enabled-profiling>
        native-optimized
)
if (APPLE AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_link_libraries(chroma-replace PRIVATE ${VISION_FRAMEWORK} ${COREGRAPHICS_FRAMEWORK} ${COREVIDEO_FRAMEWORK}
                                                 ${FOUNDATION_FRAMEWORK})
endif()


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
if (BUILD_UNIT_TESTS AND Threads_FOUND AND GTest_FOUND)
    add_executable(gtest_vision
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_vision.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/chroma_replace.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/frame_spec.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/matting.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/protect_spec.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/subject_mask.mm
    )
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/source/subject_mask.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc;-Wno-error;-Wno-deprecated-declarations")
    target_include_directories(gtest_vision
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(gtest_vision
        PRIVATE
            ${OpenCV_LIBS}
            hobbies-basal
            enabled-debugging
            GTest::gtest
            GTest::gmock
            GTest::gtest_main
            Threads::Threads
    )
    if (APPLE AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
        target_link_libraries(gtest_vision PRIVATE ${VISION_FRAMEWORK} ${COREGRAPHICS_FRAMEWORK} ${COREVIDEO_FRAMEWORK}
                                                    ${FOUNDATION_FRAMEWORK})
    endif()
    add_test(NAME gtest_vision COMMAND gtest_vision)
endif()

# === Doxygen ===
# if (Doxygen_FOUND)
#     set(DOXYGEN_GENERATE_HTML YES)
#     set(DOXYGEN_GENERATE_MAN NO)
#     doxygen_add_docs(hobbies-vision-docs
#         ${CMAKE_CURRENT_SOURCE_DIR}/include/vision
#         COMMENT "Noise Doxygen")
# endif()

endif(OpenCV_FOUND)