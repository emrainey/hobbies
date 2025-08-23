# === Project ===
cmake_minimum_required(VERSION 3.31.0)
project(linalg-utils LANGUAGES CXX VERSION 0.7.0)  # This must match the conanfile.py!

# === Dependencies ===
find_package(OpenCV)
find_package(Doxygen)

# === Targets ===
if (OpenCV_FOUND)
add_library(hobbies-linalg-utils)
target_sources(hobbies-linalg-utils
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source/opencv.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/plot.cpp
)
target_link_libraries(hobbies-linalg-utils PUBLIC
    hobbies-linalg hobbies-geometry hobbies-fourcc ${OpenCV_LIBS})
target_include_directories(hobbies-linalg-utils
    PUBLIC
        # Generated Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        # Build Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # Install Tree
        $<INSTALL_INTERFACE:include>
        ${OpenCV_INCLUDE_DIRS}
)
set_target_properties(hobbies-linalg-utils PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

# === Demos ===
add_executable(demo_least_squares ${CMAKE_CURRENT_SOURCE_DIR}/demo/demo_least_squares.cpp)
target_include_directories(demo_least_squares PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(demo_least_squares hobbies-linalg-utils)

add_executable(demo_images ${CMAKE_CURRENT_SOURCE_DIR}/demo/demo_images.cpp)
target_include_directories(demo_images PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(demo_images hobbies-linalg-utils)

# === Doxygen ===
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-linalg-utils-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/linalg
        COMMENT "Linalg Doxygen")
endif()

endif(OpenCV_FOUND)
