from conans import ConanFile, CMake

class LinAlgConan(ConanFile):
    name = "linalg"
    version = "0.7"
    settings = "os", "arch", "compiler", "build_type"
    description = "The Linear Algebra Library (and associated lower libraries)"
    generators = "cmake", "cmake_find_package", "virtualenv"
    requires = "gtest/1.8.1", "opencv/4.5.3", "basal/0.5", "units_of_measure/0.7"
    url = ""
    homepage = ""
    source_folder = "."
    build_folder = "build"

    def package_info(self):
        self.cpp_info.libs = ['linalg', 'linalg-extra', 'linalg-utils', 'neuralnet', 'raytrace', 'geometry', 'noise']

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
