#!/usr/bin/env bash

_PATH=`pwd`/build/homebrew-llvm/projects/raytrace
export DYLD_LIBRARY_PATH=${_PATH}:`pwd`/install/lib
(cd testing && ${_PATH}/demo_sdl2 $*)
# (cd testing && ${_PATH}/demo_curses $*)
