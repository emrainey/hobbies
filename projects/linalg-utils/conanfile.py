from conans import ConanFile, CMake

class LinAlgUtilsConan(ConanFile):
    name = "linalg-utils"
    version = "0.7.0" # This must match the CMakeLists.txt!
    settings = "os", "arch", "compiler", "build_type"
    description = "The Linear Algebra Utility Library"
    generators = "cmake", "cmake_paths"
    requires = "gtest/1.8.1", \
               "opencv/4.5.3", \
               "basal/0.5.0", \
               "units_of_measure/0.7.0", \
               "fourcc/0.7.0", \
               "linalg/0.7.0", \
               "geometry/0.7.0"

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
        self.cpp_info.libs = [f'hobbies-{self.name}']
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