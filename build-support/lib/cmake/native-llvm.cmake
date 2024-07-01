set(CMAKE_SYSTEM_NAME ${CMAKE_HOST_SYSTEM_NAME})
set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_HOST_SYSTEM_PROCESSOR})

find_program(CMAKE_C_COMPILER NAMES clang REQUIRED)
find_program(CMAKE_CXX_COMPILER NAMES clang++ REQUIRED)
find_program(CMAKE_OBJDUMP NAMES llvm-objdump REQUIRED)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

add_link_options(-L/opt/homebrew/opt/libomp/lib -lomp)

if (NOT TARGET enabled-warnings)
add_library(enabled-warnings INTERFACE)
target_compile_options(enabled-warnings INTERFACE
    $<BUILD_INTERFACE:-Wall -Wextra -Werror -pedantic>
)
endif()

if (NOT TARGET enabled-debugging)
add_library(enabled-debugging INTERFACE)
target_compile_options(enabled-debugging INTERFACE -glldb)
endif()

if (NOT TARGET enabled-profiling)
add_library(enabled-profiling INTERFACE)
target_link_options(enabled-profiling INTERFACE -fprofile-instr-generate)
target_compile_options(enabled-profiling INTERFACE -fprofile-instr-generate)
endif()

if (NOT TARGET native-optimized)
# Optimizes your objects for native builds (on the build side only)
add_library(native-optimized INTERFACE)
target_compile_options(native-optimized INTERFACE
    -mcpu=native
    -march=native
)
endif()
