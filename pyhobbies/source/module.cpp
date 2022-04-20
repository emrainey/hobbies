
#include <pybind11/pybind11.h>

#include <raytrace/raytrace.hpp>

// A convienent shortcut everyone seems to use
namespace py = pybind11;

PYBIND11_MODULE(pyhobbies, module) {
    module.doc() = "The C++17 Hobby w/ Raytracer PyBind11 Interface";
    module.attr("__version__") = "dev";
    {
        // raytracing submodule
        py::module sub = module.def_submodule("raytrace");
        py::class_<raytrace::color> color(sub, "color");
        color.def(py::init<raytrace::element_type, raytrace::element_type, raytrace::element_type>())
            .def("scale", &raytrace::color::scale)
            .def("red", &raytrace::color::red)
            .def("green", &raytrace::color::green)
            .def("blue", &raytrace::color::blue)
            .def("clamp", &raytrace::color::clamp)
            .def(
                "__repr__", [](const raytrace::color& c) -> auto {
                    return "<raytrace.color = R:" + std::to_string(c.red()) + " G:" + std::to_string(c.green())
                           + " B:" + std::to_string(c.blue()) + "'>";
                });
        py::enum_<raytrace::color::space>(color, "space")
            .value("linear", raytrace::color::space::linear)
            .value("logarithmic", raytrace::color::space::logarithmic)
            .export_values();
    }
}