# hobbies

A place to collect and share all my hobby projects.

## Layout

* `build` - where the build temporaries are collected
* `install` - where the build outputs are collected
* `docs` - documentation for the various projects
* `data` - sample data files used by the projects (training data, etc)
* `build-support` - CMake modules and scripts to support the build process.
* `projects` - The set of projects
  * `audio` - a simple audio synthesis and processing library
  * `basal` - a core library of utility objects and interfaces.
  * `doorgame` - a simple Text Adventure game
  * `ecs` - an Entity Component System implementation for a simple gravity simulation
  * `fourcc` - a small image library
  * `geometry` - the 2D/3D geometry objects
  * `htm` - the start of a hierarchical temporal memory system.
  * `linalg` - the core matrix oriented math library
  * `linalg-algo` - high level math algorithms
  * `linalg-utils` - a compatibility library for using OpenCV.
  * `neuralnet` - a simple NN w/ hidden layers
  * `noise` - A 2D image noise generator, useful in textures or other means
  * `pyhobbies` - a pybind wrapper for the various things here.
  * `raytrace` - a CPU raytracer using OpenMP
  * `units_of_measure` - mechanism to create types which represent Units of Measure.
  * `vision` - A set of OpenCV vision programs that I've played with.
  * `xmmt` - SSE2/AVX512 inline assembly vector and matrix headers. (does not build on ARM64)

Check the README in each folder for it's own progress and issues.

## General Design

### Principles

* This a space to learn modern C++, so the standard used and tooling in the build will evolve as the language and the tools do.
* Keep things separate and simple to be as testable as possible while having some notion that is should perform decently.
  ** e.g. Testable First, Performance Second.
* Test should accompany every commit where possible, even if results have to be graded by ear or eye.
* Strive for high coverage.
* No commit should break the build (it may not 100% work yet but it should compile).
* Metrics drive decisions on performance. Don't prematurely optimize. Capture data and make decisions based on data.
* Use language features to drive down the occurrence of common programming problems.

### Guidelines

* All math oriented objects must implement
  * `operators == != += -= *= /= + - * / !` in the class or operators namespace as `friend`s
* Must use `basal::` functions for best floating point checks. Many visual errors in raytracing are due to simple comparisons.
* Consistent use of either read-only members or methods, i.e. `const` correctness.
* Most objects must support `basal::printable`
* Most objects must support `friend operator<<`
* Use `explicit` Constructors to prevent unanticipated auto-magic
* Use `if constexpr` for debugging.
* Always install your header files as read-only!
* Always create your generated source code as read-only!
* Only locally install binaries (`install/`). These shouldn't pollute your system.

## Developing

You can open either the `hobbies.code-workspace` or the folder itself. VSCode can configure CMake now but you'll need a compiler which supports OpenMP and the Homebrew installation of it. My `CMakeUserPresets.json` is geared towards what my system has installed, so YMMV.

## Building

### Dependencies

```bash
# On Mac OS using Homebrew
brew install cmake llvm openmp sdl2 googletest fmt lcov doxygen graphviz google-benchmark
```

### Mac OS

Due to issues with OpenMP no longer being supported on Apple Clang and GCC not being able to link (can't find libgcrt1.a), LLVM is the currently working compiler. This is actually a good thing as latest LLVM code produces faster results in raytracing.

```bash
# Building on Apple Silicon
$ cmake --workflow --list-presets
Available workflow presets:

  "unit-tests"    - Homebrew LLVM
  "profiling-run" - Homebrew LLVM with Profiling

# Build the unit tests and run them
$ cmake --workflow --preset unit-tests
```

### Conan

Conan support has been removed until I have time to read up on Conan 2.0 builds. This will make it hard to build on WSL as that was my preferred method.

## Running

While in the hobbies' root, you can run things by pre-pending the LD (DYLD) path, or by using `testing.sh` which will set the path for you.

```bash
# Running the NCurses Raytracing Console on Mac (this is too wordy!)
DYLD_LIBRARY_PATH=install/homebrew-llvm/lib:build/homebrew-llvm/projects/raytrace ./build/homebrew-llvm/projects/raytrace/demo_curses -m world_example -d QCIF -r 4 -b 5
```

## Testing Folder

I've added an explicit folder to run tests. You can run them using the testing wrapper script (it will temporarily modify your `PATH` and `LD_LIBRARY_PATH`).

```bash
# Basic
./testing.sh demo_sdl2 -m world_example
# Advanced (Stereoscopic)
./testing.sh demo_sdl2 -m world_snowman --dims CIF --aaa 128 --separation 2.0
```

## Profiling

There's two ways to run the profiling tools which have been tested on Mac OS. The CMake targets for profiling exist but are broken due to Clang issues at the moment.

```bash
# This will generate a "testing/profiling.txt" with the top 100 called functions.
# >> See the Presets to build with profiling enabled! <<
./profile_run.sh ... options ...
```

```bash
# This will generate a *-perf.pdf with the details (a call tree with % time spent)
# >> See the Presets to build with profiling enabled! <<
./profile_run.sh
```

## VS Code order of precedence processing

VSCode will take settings in the order of:

* .code-workspace
* .vscode/settings.json
* project/**/.vscode/settings.json

However, all settings have been moved to the workspace file.
