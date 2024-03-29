from conans import ConanFile, CMake

class XMMTConan(ConanFile):
    name = "xmmt"
    version = "0.7.0" # This must match the CMakeLists.txt!
    settings = "os", "arch", "compiler", "build_type"
    description = "The AVX2 acceleration library"
    generators = "cmake", "cmake_paths"
    requires = "gtest/1.8.1", "basal/0.5.0"
    url = "http://github.com/emrainey/hobbies"
    homepage = "http://github.com/emrainey/hobbies"
    license = 'Unlicense'

    def package_info(self):
        self.cpp_info.name = self.name
        self.cpp_info.libs = [f'hobbies-{self.name}']
        self.cpp_info.includedirs = ['include']

    def _configure_cmake(self) -> CMake:
        cmake = CMake(self)
        cmake.definitions[f"CMAKE_PROJECT_{self.name}_INCLUDE"] = f"{self.build_folder}/conan_paths.cmake"
        cmake.configure()
        return cmake

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()
        cmake.test()