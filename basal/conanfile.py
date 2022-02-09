"""
Simple OS Conan Recipe
"""
import os
import sys
from conans import ConanFile, CMake

class BasalConan(ConanFile):
    name = "basal"
    version = "0.5"
    settings = "os", "arch", "compiler", "build_type"
    description = "A simple library which extend base features"
    generators = "cmake"
    requires = "gtest/1.8.1"
    url = "http://github.com/emrainey/hobbies"
    homepage = "http://github.com/emrainey/hobbies"
    license = 'Unlicense'

    def package_info(self):
        self.cpp_info.name = self.name
        self.cpp_info.libs = [f'emrainey-{self.name}']
        self.cpp_info.includedirs = ['include']
        self.cpp_info.build_modules["cmake_find_package"].append(f"cmake/{self.name}-config.cmake")
        self.cpp_info.build_modules["cmake_find_package"].append(f"cmake/Find{self.name}.cmake")

    def package(self):
        """ Creates the Conan package after the build """
        cmake = CMake(self)
        cmake.configure()
        cmake.install()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.install()
        cmake.test()
