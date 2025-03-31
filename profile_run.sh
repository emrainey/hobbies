#!/bin/zsh

export CPUPROFILE_FREQUENCY=1000
export DYLD_INSERT_LIBRARIES=/opt/homebrew/Cellar/gperftools/2.16/lib/libprofiler.dylib
_PATH=`pwd`/build/native-llvm-19-profiled/projects/raytrace
export DYLD_LIBRARY_PATH=./install/lib:${_PATH}
_PRGM=demo_sdl2
_FILE=${_PRGM}-perf.pdf
export CPUPROFILE=${_PRGM}.prof

(cd testing && ${_PATH}/${_PRGM} -m world_example)
(cd testing && pprof --pdf ${_PATH}/${_PRGM} ${CPUPROFILE} > ${_FILE})