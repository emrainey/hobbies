from conans import ConanFile, CMake

class MyConanFile(ConanFile):
    name = "fourcc"
    version = "0.7"
    settings = "os", "arch", "compiler", "build_type"
    description = "The FOURCC image library"
    generators = "cmake", "cmake_find_package", "virtualenv"
    requires = "gtest/1.8.1", "basal/0.5"
    url = ""
    homepage = ""
    source_folder = "."
    build_folder = "build"

    def package_info(self):
        self.cpp_info.libs = ['emrainey-fourcc']

    def package(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.install()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.install()
        cmake.test()
