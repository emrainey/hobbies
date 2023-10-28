"""
Basal Conan Recipe
"""
import os
import sys
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake, cmake_layout

class BasalConan(ConanFile):
    name = "basal"
    version = "0.5.0" # This must match the CMakeLists.txt!
    settings = "os", "arch", "compiler", "build_type"
    description = "A simple library which extend base features"
    url = "http://github.com/emrainey/hobbies"
    homepage = "http://github.com/emrainey/hobbies"
    license = 'Unlicense'
    options = {
        'shared': [False, True]
    }
    default_options = {
        'shared': False
    }

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "cmake/*", "source/*", "test/*", "include/*"

    def requirements(self):
        self.requires("gtest/1.8.1")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.test()

    def package(self):
        cmake = CMake(self)
        cmake.install()

