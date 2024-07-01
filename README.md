# hobbies

A place to collect and share all my hobby projects.

* `build` - where the build temporaries are collected
* `install` - where the build outputs are collected
* `projects` - The set of projects
  * `basal` - a core library of utility objects and interfaces.
  * `linalg` - the core matrix oriented math library
  * `linalg-algo` - high level math algorithms
  * `linalg-utils` - a compatibility library for using OpenCV.
  * `geometry` - the 2D/3D geometry objects
  * `noise` - A 2D image noise generator, useful in textures or other means
  * `fourcc` - a small image library
  * `neuralnet` - a simple NN w/ hidden layers
  * `raytrace` - a fairly simple raytracer
  * `htm` - the start of a hierarchical temporal memory system.
  * `units_of_measure` - mechanism to create types which represent Units of Measure.
  * `xmmt` - SSE2/AVX512 inline assembly vector and matrix headers. (does not build on ARM64)

Check the README in each folder for it's own progress and issues.

## General Design Principles

* All math oriented objects must implement
  * `operators == != += -= *= /= + - * / !` in the class or operators namespace as `friend`s
* Must use `basal::equals` for best float checks
* Consistent use of either read-only members or methods.
* `const` correctness.
* Must support `basal::printable`
* Must support `friend operator<<`
* Use `explicit` Constructors to prevent unanticipated auto-magic
* Use `if constexpr` for debugging.
* Always install your header files as read-only!
* Always create your generated source code as read-only!
* Only locally install binaries (`install/`). These shouldn't pollute your system.

## Developing

You can open either the `hobbies.code-workspace` or the folder itself. VSCode can configure CMake now but you'll need a compiler which supports OpenMP and the Homebrew installation of it.

## Building

Due to issues with OpenMP no longer being supported on Apple Clang and GCC not being able to link (can't find libgcrt1.a), LLVM 18 is the currently working compiler.

```bash
# Building on Apple Silicon
cmake --preset native-llvm-18
cmake --build build/native-llvm-18
```

### Conan

Conan support has been removed until I have time to read up on Conan 2.0 builds. This will make it hard to build on WSL as that was my preferred method.

## Running

While in the hobbies' root, you can run things by pre-pending the LD (DYLD) path.

```bash
# Running the NCurses Raytracing Console on Mac
DYLD_LIBRARY_PATH=install/lib:build/projects/raytrace ./build/projects/raytrace/demo_curses -m libworld_example.dylib
```

## Testing Folder

I've added an explicit folder to run tests. You can run them using the testing wrapper script (it will temporarily modify your `PATH` and `LD_LIBRARY_PATH`).

```bash
# Basic
./testing.sh demo_curses -m world_example
# Advanced (Stereoscopic)
./testing.sh demo_sdl2 -m world_snowman --dims CIF --aaa 128 --separation 2.0
```

## Profiling

There's two ways to run the profiling tools which have been tested on Mac OS.

```bash
# This will generate a "testing/profiling.txt" with the top 100 called functions.
./raytrace.sh ... options ...
```

```bash
# This will generate a *-perf.pdf with the details (a call tree with % time spent)
./profile_run.sh
```

## VS Code order of precedence processing

VSCode will take settings in the order of:

* .code-workspace
* .vscode/settings.json
* project/**/.vscode/settings.json

However, all settings have been moved to the workspace file.
