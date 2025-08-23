# === Project ===
cmake_minimum_required(VERSION 3.31.0)
project(pyhobbies LANGUAGES CXX VERSION 0.7.0)  # This must match the conanfile.py!

# === Dependencies ===
find_package(Python COMPONENTS Interpreter Development)
find_package(pybind11 CONFIG)

# === Targets ===
if (pybind11_FOUND)
pybind11_add_module(pyhobbies SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/source/module.cpp
)
target_include_directories(pyhobbies
    PUBLIC
        # Generated Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        # Build Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # Install Tree
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(pyhobbies
    PUBLIC hobbies-raytrace hobbies-basal pybind11::pybind11
)
pybind11_extension(pyhobbies)
pybind11_strip(pyhobbies)

install(TARGETS pyhobbies
    DESTINATION lib
)

endif()
