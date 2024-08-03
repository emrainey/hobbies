_PATH=`pwd`/build/native-llvm-18/projects/raytrace
export DYLD_LIBRARY_PATH=${_PATH}:`pwd`/install/lib
CLT=/Library/Developer/CommandLineTools/usr/bin/
touch testing/default.profraw
(cd testing && \
 rm default.profraw && \
 ${_PATH}/demo_sdl2 $* && \
 ${CLT}/llvm-profdata show --detailed-summary --topn=100 --output=profiling_mangled.txt default.profraw && \
 cat profiling_mangled.txt | c++filt -n  > profiling.txt )
