#!/usr/bin/env bash

export CPUPROFILE_FREQUENCY=1000
export DYLD_INSERT_LIBRARIES=/opt/homebrew/Cellar/gperftools/2.16/lib/libprofiler.dylib
_PATH=`pwd`/build/homebrew-llvm-profiled/projects/raytrace
export DYLD_LIBRARY_PATH=./install/lib:${_PATH}
_PRGM=demo_sdl2
_FILE=${_PRGM}-perf.pdf
export CPUPROFILE=${_PRGM}.prof

#  ${CLT}/llvm-profdata show --detailed-summary --topn=100 --output=profiling_mangled.txt default.profraw && \
#  cat profiling_mangled.txt | c++filt -n  > profiling.txt \

(cd testing && ${_PATH}/${_PRGM} -m world_example)
(cd testing && pprof --pdf ${_PATH}/${_PRGM} ${CPUPROFILE} > ${_FILE})