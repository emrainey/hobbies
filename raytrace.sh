export DYLD_LIBRARY_PATH=./build/raytrace:./install/lib
rm default.profraw
# ./build/raytrace/demo_sdl2 $*
# ./build/raytrace/demo_curses $*
./build/raytrace/demo_opencv_gui $*
CLT=/Library/Developer/CommandLineTools//usr/bin/
${CLT}/llvm-profdata show --detailed-summary --topn=100 --output=profiling_mangled.txt default.profraw
cat profiling_mangled.txt | c++filt -n  > profiling.txt
