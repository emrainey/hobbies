include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FETCHCONTENT_QUIET OFF)


# Function to print variables with a given prefix
function(print_variables_with_prefix prefix)
    get_cmake_property(vars VARIABLES)

    foreach(var ${vars})
        if("${var}" MATCHES "^${prefix}")
            message(STATUS "${var} = ${${var}}")
        endif()
    endforeach()
endfunction()

# function to find all targets with a given prefix
function(print_targets_with_prefix prefix directory)
    get_property(all_targets DIRECTORY ${directory} PROPERTY BUILDSYSTEM_TARGETS)

    foreach(target ${all_targets})
        if("${target}" MATCHES "^${prefix}")
            message(STATUS "Target: ${target}")
        endif()
    endforeach()

    get_property(all_targets DIRECTORY ${directory} PROPERTY IMPORTED_TARGETS)

    foreach(target ${all_targets})
        if("${target}" MATCHES "^${prefix}")
            message(STATUS "Imported Target: ${target}")
        endif()
    endforeach()
endfunction()

# --- Fetch FTXUI --------------------------------------------------------------
FetchContent_Declare(ftxui
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ftxui
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(ftxui)
print_variables_with_prefix("ftxui_")
print_targets_with_prefix("ftxui_" ${ftxui_SOURCE_DIR})

# --- Fetch SIMDe --------------------------------------------------------------
FetchContent_Declare(simde
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/simde
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(simde)
print_variables_with_prefix("simde_")
# print_targets_with_prefix("simde_" ${simde_SOURCE_DIR})

# SIMDe is header-only; provide a stable interface target for downstream links.
if(NOT TARGET simde)
    add_library(simde INTERFACE)
endif()
target_include_directories(simde INTERFACE ${simde_SOURCE_DIR})

# ------------------------------------------------------------------------------
# Googletest now is called gtest and gmock
FetchContent_Declare(gtest
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/googletest
    EXCLUDE_FROM_ALL
)
# On macOS, force googletest to use libc++ instead of libstdc++
if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++" CACHE STRING "" FORCE)
endif()
FetchContent_MakeAvailable(gtest) # This will define the gtest and gmock targets
print_variables_with_prefix("gtest_")
print_targets_with_prefix("gtest_" ${gtest_SOURCE_DIR})

# # ------------------------------------------------------------------------------
# FetchContent_Declare(googlebenchmark
#         GIT_REPOSITORY https://github.com/google/benchmark.git
#         GIT_TAG main) # need main for benchmark::benchmark
# FetchContent_MakeAvailable(googlebenchmark)
# print_variables_with_prefix("googlebenchmark_")
# print_targets_with_prefix("googlebenchmark_" ${googlebenchmark_SOURCE_DIR})
# # ------------------------------------------------------------------------------

# # ------------------------------------------------------------------------------
# FetchContent_Declare(pybind11
#         GIT_REPOSITORY https://github.com/pybind/pybind11.git
#         GIT_TAG main) # need main for benchmark::benchmark
# FetchContent_MakeAvailable(pybind11)
# print_variables_with_prefix("pybind11_")
# print_targets_with_prefix("pybind11_" ${pybind11_SOURCE_DIR})
# # ------------------------------------------------------------------------------
