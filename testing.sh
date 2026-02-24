#!/usr/bin/env bash

PRESET=homebrew-llvm

# Enforce that the build worked and installed
cmake --build build/${PRESET} --target install
echo "============="
export DYLD_LIBRARY_PATH=`pwd`/install/${PRESET}/lib:${DYLD_LIBRARY_PATH}
echo "DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}"
echo "============="
export LD_LIBRARY_PATH=`pwd`/install/${PRESET}/lib:${LD_LIBRARY_PATH}
echo "LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"
echo "============="
export PATH=`pwd`/install/${PRESET}/bin:${PATH}
echo "PATH=${PATH}"
echo "============="
(cd testing && $*)