# === Project ===
cmake_minimum_required(VERSION 3.31.0)
project(fractals LANGUAGES CXX VERSION 0.1.0)  # This must match the conanfile.py!

# === Dependencies ===
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(SDL2)
find_package(Doxygen)

# === Targets ===
add_library(hobbies-fractals
    ${CMAKE_CURRENT_SOURCE_DIR}/source/fractals.cpp
)
target_link_libraries(hobbies-fractals
    PUBLIC
        hobbies-basal
        hobbies-geometry
        hobbies-fourcc
    PRIVATE
        enabled-debugging
        $<$<BOOL:${USE_PROFILING}>:enabled-profiling>
        $<$<BOOL:${USE_COVERAGE}>:enabled-coverage>
        enabled-warnings
        native-optimized
)
target_include_directories(hobbies-fractals
    PUBLIC
        # Generated Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        # Build Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # Install Tree
        $<INSTALL_INTERFACE:include>
)
set_target_properties(hobbies-fractals PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)
install(TARGETS hobbies-fractals
    EXPORT fractals-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# === Demos ===
if (SDL2_FOUND)
add_executable(demo_fractals ${CMAKE_CURRENT_SOURCE_DIR}/demo/main.cpp)
target_link_libraries(demo_fractals PUBLIC ${SDL2_LIBRARIES} hobbies-fractals)
install(TARGETS demo_fractals
    EXPORT fractals-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
endif(SDL2_FOUND)

# === Doxygen ===
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-fractals-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/fractals
        COMMENT "Fractals Doxygen")
endif()