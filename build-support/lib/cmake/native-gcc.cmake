set(CMAKE_SYSTEM_NAME ${CMAKE_HOST_SYSTEM_NAME})
set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_HOST_SYSTEM_PROCESSOR})

message(NOTICE "Turn off poor features")
add_compile_options(-Wno-deprecated-register -Wno-maybe-uninitialized)

add_library(enabled-warnings INTERFACE)
target_compile_options(enabled-warnings INTERFACE
    $<BUILD_INTERFACE:-Wall -Wextra -Werror -pedantic -Wno-maybe-uninitialized>
)

add_library(enabled-debugging INTERFACE)
target_compile_options(enabled-debugging INTERFACE -g -ggdb)

add_library(enabled-profiling INTERFACE)
target_link_options(enabled-profiling INTERFACE -pg)
target_compile_options(enabled-profiling INTERFACE -pg)

# Optimizes your objects for native builds (on the build side only)
add_library(native-optimized INTERFACE)
target_compile_options(native-optimized INTERFACE -mcpu=apple-m3)
