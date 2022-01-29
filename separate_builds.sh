#!/bin/bash
set -x
set -e

CLEAN=0
JOBS=16
VERBOSE=OFF
while [[ $# -gt 0 ]]; do
    case $1 in
        -rm)
            CLEAN=1
        ;;
        -s)
            JOBS=1
        ;;
        -v)
            VERBOSE=ON
        ;;
        *)
        ;;
    esac
    shift
done

# The local install location to test the build
INSTALL_ROOT=`pwd`/install
mkdir -p ${INSTALL_ROOT}

# Which packages to build in which order
PKGS=(basal units_of_measure fourcc)

# Cycle over each package and build it
for pkg in "${PKGS[@]}"; do
    if [[ CLEAN -eq 1 ]]; then
        rm -rf $pkg/build
        mkdir -p $pkg/build
    fi
    cmake -B $pkg/build -S $pkg \
        -DCMAKE_INSTALL_PREFIX:PATH=${INSTALL_ROOT} \
        -DCMAKE_PREFIX_PATH:PATH=${INSTALL_ROOT} \
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=${VERBOSE} \
        -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
        -DCMAKE_BUILD_TYPE:STRING=Release \
        -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/clang \
        -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/clang++
    cmake --build $pkg/build -j${JOBS}
    cmake --build $pkg/build --target install
done