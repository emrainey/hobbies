from conans import ConanFile, CMake

class UnitsOfMeasureConan(ConanFile):
    name = "units_of_measure"
    version = "0.7"
    settings = "os", "arch", "compiler", "build_type"
    description = "A library for defining ISO/IEC"
    generators = "cmake", "cmake_find_package", "virtualenv"
    requires = "gtest/1.8.1"
    pips = ['jinja2']
    url = ""
    homepage = ""
    source_folder = "."
    build_folder = "build"

    def system_requirements(self):
        import pip
        if hasattr(pip, "main"):
            pip.main(self.pips)
        else:
            from pip._internal import main
            main(self.pips)

    def package_info(self):
        self.cpp_info.libs = ['units_of_measure']

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
