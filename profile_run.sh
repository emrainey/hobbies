#!/bin/zsh

export CPUPROFILE_FREQUENCY=1000
export DYLD_INSERT_LIBRARIES=/usr/local/Cellar/gperftools/2.10/lib/libprofiler.dylib
_PATH=./build/raytrace
export DYLD_LIBRARY_PATH=./install/lib:${_PATH}
_PRGM=demo_curses
_FILE=${_PRGM}.pdf
export CPUPROFILE=${_PRGM}.prof

${_PATH}/${_PRGM} -m world_example -d 1
pprof --pdf ${_PATH}/${_PRGM} ${CPUPROFILE} > ${_FILE}