#!/usr/bin/env bash

unset PRJ_PATH
for prj in `ls -1 build/projects`; do
    #echo "${prj}"
    PRJ_PATH=${PRJ_PATH}:`pwd`/build/projects/${prj}
done

export DYLD_LIBRARY_PATH=${PRJ_PATH}
export PATH=${PATH}:${PRJ_PATH}
echo "============="
echo "DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}"
echo "============="
echo "PATH=${PATH}"
(cd testing && $*)