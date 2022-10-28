from conans import ConanFile, CMake
import os


class UnitsOfMeasureConan(ConanFile):
    name = "units_of_measure"
    version = "0.7.0"  # This must match the CMakeLists.txt!
    settings = "os", "arch", "compiler", "build_type"
    description = "A library for defining ISO/IEC standard units of measure"
    generators = "cmake", "cmake_paths"
    requires = "gtest/1.8.1"
    url = "http://github.com/emrainey/hobbies"
    homepage = "http://github.com/emrainey/hobbies"
    license = 'Unlicense'
    options = {
        'shared': [False, True]
    }
    default_options = {
        'shared': False
    }

    def package_info(self):
        self.cpp_info.name = self.name
        self.cpp_info.libs = ['hobbies-uom']
        self.cpp_info.includedirs = ['include']

    def _configure_cmake(self) -> CMake:
        cmake = CMake(self)
        cmake.definitions[f"CMAKE_PROJECT_{self.name}_INCLUDE"] = f"{self.build_folder}/conan_paths.cmake"
        # since we have a 'shared', BUILD_SHARED_LIBS will automatically be set
        cmake.configure()
        return cmake

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()
        cmake.test()