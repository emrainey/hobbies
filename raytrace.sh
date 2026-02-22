#!/usr/bin/env bash

_PATH=`pwd`/build/homebrew-llvm/projects/raytrace
export DYLD_LIBRARY_PATH=${_PATH}:`pwd`/install/lib
(cd testing && ${_PATH}/demo_sdl2 $*)
# (cd testing && ${_PATH}/demo_curses $*)
# (cd testing && ${_PATH}/demo_functions)
# (cd testing && ${_PATH}/demo_opencv_gui $*)
# (cd testing && ${_PATH}/demo_random_tuner)
# (cd testing && ${_PATH}/demo_sphere_surfaces)
