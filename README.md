# hobbies
A place to collect and share all my hobby projects.

* `basal` - a core library of utility objects and interfaces.
* `linalg` - the core matrix oriented math library
  * `linalg-utils` - a compatibility library for using OpenCV.
* `geometry` - the 2D/3D geometry objects
* `noise` - A 2D image noise generator, useful in textures or other means
* `fourcc` - a small image library
* `neuralnet` - a simple NN w/ hidden layers
* `raytrace` - a fairly simple raytracer
* `htm` - the start of a hierarchical temporal memory system.
* `units_of_measure` - mechanism to create types which represent Units of Measure. 

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

You can open either the `hobbies.code-workspace` or the folder itself. Currently VSCode is not intelligent about running _configure_ in order.

## Building

Currently you can run the `./separate_builds.sh -rm -s` to individually config, build, and install each package. All the artifacts will end up in a new `install/` folder.

### Conan 

Hobbies can be build using Conan (http://conan.io). In Windows for Linux, and older versions of Linux you can use the `wsl_conanfile.txt` to setup a build environment and then call the `./separate_build.sh -c` to use Conan to communicate between projects. 

```bash
conan install wsl_conanfile.txt
source ./activate.sh
./seperate_builds.sh -rm -c
source ./deactivate.sh
```

## Running

While in the hobbies' root, you can run things by pre-pending the LD (DYLD) path.

```bash
# Running the NCurses Raytracing Console on Mac
DYLD_LIBRARY_PATH=install/lib:raytrace/build ./raytrace/build/demo_curses -m libworld_example.dylib
```

## Hobbies TODO

* TODO Enable Multi-Workspace CMake in VSCode (https://devblogs.microsoft.com/cppblog/visual-studio-code-cmake-tools-extension-multi-root-workspaces-and-file-based-api/)
* TODO Enable Building with a Single Parent CMakeLists.txt for faster builds.
