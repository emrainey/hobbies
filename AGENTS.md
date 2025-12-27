# AGENTS.md

## Building C++ Projects with Homebrew LLVM and CMake

This project uses CMakePresets to manage build configurations.

* `cmake --workflow --list-presets` to see available presets.
* `cmake --workflow --preset <preset-name>` to configure and build using a specific preset.

For Unit Tests, use either the

* `cmake --workflow --preset unit-tests` to build and run tests in one step. This will however try to run many render tests which are only evaluated by looking at them.
* `cmake --workflow --preset unit-tests-build` to only build the tests. You can then run them manually from the build directory.

To Configure the CMake Build and then call tests directly, use:

* `cmake --preset homebrew-llvm` - to configure the build with the Homebrew LLVM toolchain.
* `cmake --build --preset build-with-homebrew-llvm` - to build the project.

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
