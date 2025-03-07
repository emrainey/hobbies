
#include <pybind11/pybind11.h>

#include <raytrace/raytrace.hpp>

// A convenient shortcut everyone seems to use
namespace py = pybind11;

PYBIND11_MODULE(pyhobbies, module) {
    module.doc() = "The C++17 Hobby w/ Raytracer PyBind11 Interface";
    module.attr("__version__") = "dev";
    {
        // raytracing submodule
        py::module sub = module.def_submodule("raytrace");
        py::class_<raytrace::color> color(sub, "color");
        color.def(py::init<raytrace::precision, raytrace::precision, raytrace::precision>())
            .def("scale", &raytrace::color::scale)
            // .def("red", &raytrace::color::red)
            // .def("green", &raytrace::color::green)
            // .def("blue", &raytrace::color::blue)
            .def("clamp", &raytrace::color::clamp)
            .def("__repr__", [](raytrace::color const& c) -> auto {
                return "<raytrace.color = R:" + std::to_string(c.red()) + " G:" + std::to_string(c.green())
                       + " B:" + std::to_string(c.blue()) + ">";
            });
        // py::class_<raytrace::point> point(sub, "point");
        // point.def(py::init<precision, precision, precision>())
        //     .def("x", &raytrace::point::x)
        //     .def("y", &raytrace::point::y)
        //     .def("z", &raytrace::point::z);
        py::enum_<raytrace::color::space>(color, "space")
            .value("linear", raytrace::color::space::linear)
            .value("logarithmic", raytrace::color::space::logarithmic)
            .export_values();

        // py::class_<raytrace::objects::sphere>(sub, "sphere")
        //     .def(py::init<raytrace::point, raytrace::precision>())
        //     .def("position", &static_cast<raytrace::objects::sphere::position)
        //     .def("move_by", &raytrace::objects::sphere::move_by)
        //     .def("material", &raytrace::objects::sphere::material);
    }
}
