#!/usr/bin/env bash

unset PRJ_PATH
for prj in `ls -1 build/native-llvm-18/projects`; do
    #echo "${prj}"
    PRJ_PATH=${PRJ_PATH}:`pwd`/build/native-llvm-18/projects/${prj}
done

export DYLD_LIBRARY_PATH=${PRJ_PATH}
export PATH=${PATH}:${PRJ_PATH}
echo "============="
echo "DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}"
echo "============="
echo "PATH=${PATH}"
(cd testing && $*)