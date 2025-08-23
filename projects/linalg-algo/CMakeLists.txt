# === Project ===
cmake_minimum_required(VERSION 3.31.0)
project(linalg-algo LANGUAGES CXX VERSION 0.7.0)  # This must match the conanfile.py!

# === Dependencies ===
find_package(OpenCV)

# === Targets ===
add_library(hobbies-linalg-algo
    ${CMAKE_CURRENT_SOURCE_DIR}/source/kmeans.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/dkf.cpp
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

# === Demos ===
if (OpenCV_FOUND)
add_executable(demo_kmeans ${CMAKE_CURRENT_SOURCE_DIR}/demo/demo_kmeans.cpp)
target_link_libraries(demo_kmeans hobbies-linalg-algo ${OpenCV_LIBS})
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
