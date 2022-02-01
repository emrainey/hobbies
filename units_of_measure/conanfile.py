from conans import ConanFile, CMake
import os

class UnitsOfMeasureConan(ConanFile):
    name = "units_of_measure"
    version = "0.7"
    settings = "os", "arch", "compiler", "build_type"
    description = "A library for defining ISO/IEC standard units of measure"
    generators = "cmake", "cmake_paths"
    requires = "gtest/1.8.1"
    url = "http://github.com/emrainey/hobbies"
    homepage = "http://github.com/emrainey/hobbies"
    license = 'Unlicense'

    def package_info(self):
        self.cpp_info.name = self.name
        self.cpp_info.libs = ['emrainey-uom']
        self.cpp_info.includedirs = ['include']
        self.cpp_info.build_modules["cmake_find_package"].append(f"cmake/{self.name}-config.cmake")
        self.cpp_info.build_modules["cmake_find_package"].append(f"cmake/Find{self.name}.cmake")

    def package(self):
        cmake = CMake(self)
        cmake.definitions["USE_CONAN"] = "TRUE"
        cmake.configure()
        cmake.install()

    def build(self):
        cmake = CMake(self)
        cmake.definitions["USE_CONAN"] = "TRUE"
        cmake.configure()
        cmake.build()
        cmake.install()
        cmake.test()
