message("======== CALLED FROM ${CMAKE_CURRENT_LIST_FILE} from ${CMAKE_PARENT_LIST_FILE}")
set(CMAKE_SYSTEM_NAME ${CMAKE_HOST_SYSTEM_NAME})
set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_HOST_SYSTEM_PROCESSOR})

find_program(CMAKE_C_COMPILER NAME gcc REQUIRED)
find_program(CMAKE_CXX_COMPILER NAME g++ REQUIRED)
find_program(CMAKE_LD NAME ld REQUIRED)

if (APPLE AND CMAKE_CXX_COMPILER MATCHES "/opt/homebrew/")
message(WARNING "Homebrew GCC detected on macOS; this toolchain is not supported due to libc++/libstdc++ ABI mismatches and missing profiling runtime objects.")
message(FATAL_ERROR "Please use the Homebrew LLVM/Clang toolchain preset on macOS.")
endif()

# message(NOTICE "Turn off poor features")
# add_compile_options(-Wno-deprecated-register -Wno-maybe-uninitialized)

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
target_link_options(enabled-profiling INTERFACE $<$<PLATFORM_ID:Linux>:-pg>)
target_compile_options(enabled-profiling INTERFACE
    $<$<PLATFORM_ID:Linux>:-pg>
    $<$<PLATFORM_ID:Darwin>:-fno-omit-frame-pointer>
)
endif()

if (NOT TARGET enabled-coverage)
add_library(enabled-coverage INTERFACE)
target_compile_options(enabled-coverage INTERFACE -fprofile-arcs -ftest-coverage)
endif()

if (NOT TARGET native-optimized)
# Optimizes your objects for native builds (on the build side only)
add_library(native-optimized INTERFACE)
target_compile_options(native-optimized INTERFACE
    -mtune=native
    $<$<PLATFORM_ID:Darwin>:-mcpu=native>
    $<$<PLATFORM_ID:Linux>:-march=native> # does not work on GCC-15 Homebrew for some reason?
)
endif()

function(coverage_target TARGET)
    add_custom_target(coverage-${TARGET}
        COMMAND gcovr --root ${CMAKE_SOURCE_DIR} --object-directory ${CMAKE_BINARY_DIR} --xml-pretty --xml-coverage ${CMAKE_BINARY_DIR}/coverage-${TARGET}.xml
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endfunction()