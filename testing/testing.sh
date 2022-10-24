#!/bin/bash
echo "Args $*"
PROJECTS=(noise raytrace htm doorgame)
for prj in "${PROJECTS[@]}"; do
    PATH=${PATH}:${PWD}/../build/${prj}
    LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${PWD}/../build/${prj}
    DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${PWD}/../build/${prj}
done
conan install .
source ./activate.sh
echo "PATH=${PATH}"
echo "LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"
echo "DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}"
$*
source ./deactivate.sh
