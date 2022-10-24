#!/bin/bash
echo "Args $*"
export PATH=$PATH:../.conan/raytrace/build
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../.conan/raytrace/build:../.conan/noise/build
conan install .
source ./activate.sh
#demo_curses $*
#../.conan/noise/build/demo_fbm_tuner $*
$*
source ./deactivate.sh
