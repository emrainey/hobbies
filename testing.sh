#!/usr/bin/env bash

if [[ -z ${PRJ_ROOT} ]]; then
    PRJ_ROOT=`pwd`/build/homebrew-llvm/projects
fi
unset PRJ_PATH
for prj in `ls -1 ${PRJ_ROOT}`; do
    #echo "${prj}"
    PRJ_PATH=${PRJ_PATH}:${PRJ_ROOT}/${prj}
done

export DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${PRJ_PATH}
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${PRJ_PATH}
export PATH=${PATH}:${PRJ_PATH}
echo "============="
echo "DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}"
echo "============="
echo "LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"
echo "============="
echo "PATH=${PATH}"
(cd testing && ../$*)