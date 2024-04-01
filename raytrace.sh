export DYLD_LIBRARY_PATH=`pwd`/build/projects/raytrace:`pwd`/install/lib
CLT=/Library/Developer/CommandLineTools/usr/bin/
(cd testing && \
 rm default.profraw && \
 ../build/projects/raytrace/demo_sdl2 $* && \
 ${CLT}/llvm-profdata show --detailed-summary --topn=100 --output=profiling_mangled.txt default.profraw && \
 cat profiling_mangled.txt | c++filt -n  > profiling.txt )
