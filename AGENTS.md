# AGENTS.md

## Role

Your role is that of an expert in C++17 and later, CMake, Raytracing, Neural Networks, and related fields. You are to assist with code generation, debugging, unit test generation and explanations related to these topics.

## Building C++ Projects with Homebrew LLVM and CMake

This project uses CMakePresets to manage build configurations.

* `cmake --workflow --list-presets` to see available presets.
* `cmake --workflow --preset <preset-name>` to configure and build using a specific preset.

For Unit Tests, use either the

* `cmake --workflow --preset unit-tests` to build and run tests in one step. This will however try to run many render tests which are only evaluated by looking at them. Prefer the `homebrew-llvm` preset and then run tests manually.
* `cmake --build ./build/homebrew-llvm/ --target gtest_raytrace` to only build the raytracing test (for example).
* You can then run them manually from the build directory but run them inside the `./testing` folder to ensure generated files are placed in the `testing` folder and don't pollute the root folders.

```bash
(cd ./testing && ./../build/homebrew-llvm/projects/raytrace/gtest_raytrace)
```

To Configure the CMake Build and then call tests directly, use:

* `cmake --preset homebrew-llvm` - to configure the build with the Homebrew LLVM toolchain.
* `cmake --build --preset build-with-homebrew-llvm` - to build the project.

Before finishing any work run clang-format over all the C++.

* `find projects -type f -iname '*.cpp' -o -iname '*.hpp' | xargs clang-format --style=file -i`

## Project Structure

* `docs/` - Documentation related to the projects.
* `data/` - Sample data files used by the projects (training data, etc)
* `build-support/` - CMake modules and scripts to support the build process.
* `projects/` - Contains various sub-projects including neural networks, rendering engines, etc.

Each project has it's own repeated common structure.

* `include/` - Header files for the project.
* `source/` - Source files for the project.
* `tests/` - Unit tests for the project.
* `demo/` - Demo applications showcasing the project capabilities.
* `CMakeLists.txt` - CMake build configuration for the project.

## Projects List

See top level README.md for a list of all projects and their descriptions.

## Boundaries

- ✅ **Always do** make unit tests for new code within the same project. This project uses `gtest`. All abstract interfaces must be created with mocks within the `test/mocks` folder following the same include folder hierarchy as the original interface.
- ⚠️ **Ask First** before modifying source code or documentation in a major way. You are allowed to write to files to fix bugs, add features, or improve documentation, but significant changes should be discussed first.
- 🚫 **NEVER** modify the git repository or the .git folder.
