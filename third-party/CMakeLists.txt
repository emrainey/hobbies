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
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    GIT_REPOSITORY https://github.com/arthursonzogni/ftxui.git
    GIT_TAG        v5.0.0
    GIT_PROGRESS   TRUE
    GIT_SHALLOW    TRUE
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(ftxui)

# # ------------------------------------------------------------------------------
# # Googletest now is called gtest and gmock
# FetchContent_Declare(
#     gtest
#     DOWNLOAD_EXTRACT_TIMESTAMP ON
#     GIT_REPOSITORY https://github.com/google/googletest/
#     GIT_TAG v1.17.0
# )
# # On macOS, force googletest to use libc++ instead of libstdc++
# if(APPLE)
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++" CACHE STRING "" FORCE)
# endif()
# FetchContent_MakeAvailable(gtest) # This will define the gtest and gmock targets
# print_variables_with_prefix("gtest_")
# print_targets_with_prefix("gtest_" ${gtest_SOURCE_DIR})

# # ------------------------------------------------------------------------------
# FetchContent_Declare(googlebenchmark
#         GIT_REPOSITORY https://github.com/google/benchmark.git
#         GIT_TAG main) # need main for benchmark::benchmark
# FetchContent_MakeAvailable(googlebenchmark)
# print_variables_with_prefix("googlebenchmark_")
# print_targets_with_prefix("googlebenchmark_" ${googlebenchmark_SOURCE_DIR})
# # ------------------------------------------------------------------------------
