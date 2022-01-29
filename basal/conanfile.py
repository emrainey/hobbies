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
    description = "A simple OS extension library"
    generators = "cmake"
    requires = ""
    url = "" # "http://github.com/emrainey/hobbies"
    homepage = "" #"http://github.com/emrainey/hobbies"
    license = 'Unlicense'

    # No source() as we don't download the git repo.
    def export_sources(self):
        print("Running from ", os.getcwd())
        self.copy("*.hpp", dst="include", src="include")
        self.copy("*.cpp", dst="source", src="source")
        self.copy("*.cpp", dst="test", src="test")
        self.copy("CMakeLists.txt", dst=".", src=".")

    def package_info(self):
        self.cpp_info.name = "basal"
        """ Used to specify the CMake Generator Input """
        self.cpp_info.components["lib"].names["cmake"] = "basal"
        self.cpp_info.components["lib"].includedirs = ["include"]
        # self.cpp_info.libs = None
        # self.cpp_info.systemlibs = None
        self.cpp_info.components["lib"].libdirs = ['lib']
        self.cpp_info.components["lib"].resdirs = ['res']
        self.cpp_info.components["lib"].bindirs = ['bin']
        self.cpp_info.components["lib"].srcdirs = ['source']
        # self.cpp_info.build_modules['cmake'].append("")
        # self.cpp_info.build_modules['cmake_find_package'].append("")
        self.cpp_info.components["lib"].defines = []
        self.cpp_info.components["lib"].cflags = []
        self.cpp_info.components["lib"].cxxflags = []
        self.cpp_info.components["lib"].sharedlinkflags = []
        self.cpp_info.components["lib"].exelinkflags = []

        self.cpp_info.components["gtest_basal"].names["cmake"] = "gtest_basal"
        self.cpp_info.components["gtest_basal"].includedirs = ["include"]
        # self.cpp_info.libs = None
        # self.cpp_info.systemlibs = None
        self.cpp_info.components["gtest_basal"].libdirs = ['lib']
        self.cpp_info.components["gtest_basal"].resdirs = ['res']
        self.cpp_info.components["gtest_basal"].bindirs = ['bin']
        self.cpp_info.components["gtest_basal"].srcdirs = ['test']
        # self.cpp_info.build_modules['cmake'].append("")
        # self.cpp_info.build_modules['cmake_find_package'].append("")
        self.cpp_info.components["gtest_basal"].defines = []
        self.cpp_info.components["gtest_basal"].cflags = []
        self.cpp_info.components["gtest_basal"].cxxflags = []
        self.cpp_info.components["gtest_basal"].sharedlinkflags = []
        self.cpp_info.components["gtest_basal"].exelinkflags = []
        # self.cpp_info.requires = None

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
        # cmake.test() # There's no tests
