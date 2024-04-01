#!/bin/zsh

export CPUPROFILE_FREQUENCY=1000
export DYLD_INSERT_LIBRARIES=/opt/homebrew/Cellar/gperftools/2.15/lib/libprofiler.dylib
_PATH=`pwd`/build/projects/raytrace
export DYLD_LIBRARY_PATH=./install/lib:${_PATH}
_PRGM=demo_curses
_FILE=${_PRGM}-perf.pdf
export CPUPROFILE=${_PRGM}.prof

(cd testing && ${_PATH}/${_PRGM} -m world_example -d 1)
(cd testing && pprof --pdf ${_PATH}/${_PRGM} ${CPUPROFILE} > ${_FILE})