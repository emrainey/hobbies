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

# === Tests ===
if (BUILD_UNIT_TESTS)
    # Found on https://discourse.cmake.org/t/possible-to-create-a-python-virtual-env-from-cmake-and-then-find-it-with-findpython3/1132/2
    # Find just the interpreter first
    find_package(Python3 REQUIRED Interpreter)
    # Print the version and create the virtual environment
    execute_process (
        COMMAND_ECHO STDOUT
        COMMAND ${Python3_EXECUTABLE} --version
        COMMAND ${Python3_EXECUTABLE} -m venv ${CMAKE_BINARY_DIR}/venv
    )
    # This mimics the activation of the venv
    set(ENV{VIRTUAL_ENV} "${CMAKE_BINARY_DIR}/venv")
    # changes the context of the search
    set(Python3_FIND_VIRTUALENV FIRST)
    # gets rid of the previous find
    unset(Python3_EXECUTABLE)
    ## Launch a new search
    find_package (Python3 COMPONENTS Interpreter Development)
    # Print the version and the install the pips
    execute_process (
        COMMAND_ECHO STDOUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND ${Python3_EXECUTABLE} --version
        COMMAND ${Python3_EXECUTABLE} -m pip install jinja2 pyyaml pytest
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    add_test(
        NAME pyhobbies-pytests
        COMMAND ${Python3_EXECUTABLE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )
    set_tests_properties(pyhobbies-pytests PROPERTIES
        ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}:${CMAKE_BINARY_DIR}/projects/raytrace:$ENV{PYTHONPATH}"
    )
endif(BUILD_UNIT_TESTS)

endif()