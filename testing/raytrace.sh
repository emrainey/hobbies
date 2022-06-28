#!/bin/bash
echo "Args $*"
export PATH=$PATH:../.conan/raytrace/build
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../.conan/raytrace/build
conan install .
source ./activate.sh
#demo_curses $*
#source ./deactivate.sh
