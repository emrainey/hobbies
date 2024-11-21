message("======== CALLED FROM ${CMAKE_CURRENT_LIST_FILE} from ${CMAKE_PARENT_LIST_FILE}")
set(CMAKE_SYSTEM_NAME ${CMAKE_HOST_SYSTEM_NAME})
set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_HOST_SYSTEM_PROCESSOR})

message(NOTICE "Turn off poor features")
add_compile_options(-Wno-deprecated-register -Wno-maybe-uninitialized)

if (NOT TARGET enabled-warnings)
add_library(enabled-warnings INTERFACE)
target_compile_options(enabled-warnings INTERFACE
    $<BUILD_INTERFACE:-Wall -Wextra -Werror -pedantic -Wno-maybe-uninitialized>
)
endif()

if (NOT TARGET enabled-debugging)
add_library(enabled-debugging INTERFACE)
target_compile_options(enabled-debugging INTERFACE -g -ggdb)
endif()

if (NOT TARGET enabled-profiling)
add_library(enabled-profiling INTERFACE)
target_link_options(enabled-profiling INTERFACE -pg)
target_compile_options(enabled-profiling INTERFACE -pg)
endif()

if (NOT TARGET enabled-coverage)
add_library(enabled-coverage INTERFACE)
target_compile_options(enabled-coverage INTERFACE -fprofile-arcs -ftest-coverage)
endif()

if (NOT TARGET native-optimized)
# Optimizes your objects for native builds (on the build side only)
add_library(native-optimized INTERFACE)
target_compile_options(native-optimized INTERFACE -mcpu=apple-m3)
endif()

function(coverage_target TARGET)
    add_custom_target(coverage-${TARGET}
        COMMAND gcovr --root ${CMAKE_SOURCE_DIR} --object-directory ${CMAKE_BINARY_DIR} --xml-pretty --xml-coverage ${CMAKE_BINARY_DIR}/coverage-${TARGET}.xml
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()