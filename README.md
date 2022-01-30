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

Check the README in each folder for it's own progress and issues.

## General Design Principles

* All math oriented objects must implement
  * `operators == != += -= *= /= + - * / !` in the class or operators namespace as `friend`s
* Must use `basal::equals` for best float checks
* Consistent use of either read-only members or methods. `const` correctness.
* Must support printable (with pointer)
* Must support `friend operator<<`
* Use `explicit` Constructors to prevent unexplained auto-magic
* Use `if constexpr` for debugging.
* Always install your header files as read-only!
* Always create your generated source code as read-only!
* Only locally install binaries (`install/`). These shouldn't pollute your system.

## Running

While in the hobbies' root, you can run things by pre-pending the LD path.

```bash
# Mac
DYLD_LIBRARY_PATH=install/lib:raytrace/build ./raytrace/build/demo_curses -m libworld_example.dylib
```