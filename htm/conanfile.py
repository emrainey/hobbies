from conans import ConanFile, CMake

class HTMConan(ConanFile):
    name = "htm"
    version = "0.1"
    settings = "os", "arch", "compiler", "build_type"
    description = "The Hierarchical Temporial Memory Library"
    generators = "cmake", "cmake_paths"
    requires = "gtest/1.8.1", "fourcc/0.7@emrainey/main"
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
        cmake = CMake(self)
        cmake.definitions[f"CMAKE_PROJECT_{self.name}_INCLUDE"] = f"{self.build_folder}/conan_paths.cmake"
        cmake.configure()
        cmake.install()

    def build(self):
        cmake = CMake(self)
        cmake.definitions[f"CMAKE_PROJECT_{self.name}_INCLUDE"] = f"{self.build_folder}/conan_paths.cmake"
        cmake.configure()
        cmake.build()
        cmake.install()
        cmake.test()