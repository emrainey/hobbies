message("======== CALLED FROM ${CMAKE_CURRENT_LIST_FILE} from ${CMAKE_PARENT_LIST_FILE}")
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

if (NOT TARGET enabled-warnings)
add_library(enabled-warnings INTERFACE)
target_compile_options(enabled-warnings INTERFACE
    $<BUILD_INTERFACE:-Wall -Wextra -Werror -pedantic -Wconversion -Wdouble-promotion -Wfloat-equal>
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

if (NOT TARGET enabled-coverage)
add_library(enabled-coverage INTERFACE)
target_compile_options(enabled-coverage INTERFACE -fprofile-arcs -ftest-coverage -fcoverage-mapping )
endif()

if (NOT TARGET native-optimized)
# Optimizes your objects for native builds (on the build side only)
add_library(native-optimized INTERFACE)
target_compile_options(native-optimized INTERFACE
    -mcpu=native
    -march=native
)
endif()

function(coverage_target TARGET)
    # set(LLVM_PROFILE_FILE "${TARGET}.profraw")
    add_custom_target(coverage-${TARGET}
        DEPENDS ${TARGET} test
        COMMAND llvm-profdata merge -sparse ${CMAKE_CURRENT_BINARY_DIR}/default.profraw -o ${CMAKE_CURRENT_BINARY_DIR}/default.profdata
        # COMMAND llvm-cov show $<TARGET_FILE:${TARGET}> -instr-profile=${CMAKE_CURRENT_BINARY_DIR}/default.profdata
        COMMAND llvm-cov report $<TARGET_FILE:${TARGET}> -instr-profile=${CMAKE_CURRENT_BINARY_DIR}/default.profdata
        COMMAND llvm-cov export $<TARGET_FILE:${TARGET}> -instr-profile=${CMAKE_CURRENT_BINARY_DIR}/default.profdata -format=lcov > ${CMAKE_CURRENT_BINARY_DIR}/default.info
        COMMAND genhtml ${CMAKE_CURRENT_BINARY_DIR}/default.info -o ${CMAKE_BINARY_DIR}/coverage/coverage-${TARGET}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endfunction()
