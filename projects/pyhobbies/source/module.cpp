
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/functional.h>

#include <raytrace/raytrace.hpp>

// A convenient shortcut everyone seems to use
namespace py = pybind11;

PYBIND11_MODULE(pyhobbies, module) {
    module.doc() = "The C++17 Hobby w/ Raytracer PyBind11 Interface";
    module.attr("__version__") = "dev";
    {
        // raytracing submodule
        // COLOR Object
        py::module submodule = module.def_submodule("raytrace");
        py::class_<raytrace::color> color_class(submodule, "color");
        color_class.def(py::init<raytrace::precision, raytrace::precision, raytrace::precision>())
            .def("scale", &raytrace::color::scale)
            .def(py::self *= float())  // scales
            .def("red", (raytrace::precision const& (raytrace::color::*)() const)&raytrace::color::red)
            .def("green", (raytrace::precision const& (raytrace::color::*)() const)&raytrace::color::green)
            .def("blue", (raytrace::precision const& (raytrace::color::*)() const)&raytrace::color::blue)
            .def("clamp", &raytrace::color::clamp)
            .def("__repr__", [](raytrace::color const& c) -> auto {
                return "<raytrace.color = R:" + std::to_string(c.red()) + " G:" + std::to_string(c.green())
                       + " B:" + std::to_string(c.blue()) + ">";
            });
        py::enum_<fourcc::Encoding>(color_class, "space")
            .value("linear", fourcc::Encoding::Linear)
            .value("logarithmic", fourcc::Encoding::GammaCorrected)
            .export_values();
        // POINT Object
        py::class_<raytrace::point> point_class(submodule, "point");
        point_class.def(py::init<raytrace::precision, raytrace::precision, raytrace::precision>())
            .def(py::self *= float())   // scale
            .def(py::self == py::self)  // equality
            // .def(py::self != py::self) // inequality
            .def("__getitem__", [](raytrace::point& pnt, size_t index) -> auto { return pnt[index]; })
            .def("__setitem__",
                 [](raytrace::point& pnt, size_t index, raytrace::precision value) { pnt[index] = value; })
            .def("__len__", [](raytrace::point& pnt) -> auto { return pnt.dimensions; })
            .def("zero", &raytrace::point::zero)
            .def("__repr__", [](raytrace::point const& pnt) -> auto {
                std::ostringstream oss;
                oss << pnt;        // Use the C++ operator<<
                return oss.str();  // Return the resulting std::string
            });

        // VECTOR Object
        py::class_<raytrace::vector> vector_class(submodule, "vector");
        vector_class.def(py::init<raytrace::precision, raytrace::precision, raytrace::precision>())
            .def(py::self *= float())   // scale
            .def(py::self /= float())   // divide
            .def(py::self += py::self)  // accumulate
            .def(py::self -= py::self)  // de-accumulate
            .def(py::self == py::self)  // equality
            .def("__add__",
                 [](raytrace::vector& a, raytrace::vector const& b) -> raytrace::vector {
                     using namespace raytrace;
                     using namespace geometry::operators;
                     raytrace::vector c;
                     c = a + b;
                     return c;
                 })
            .def("__sub__",
                 [](raytrace::vector& a, raytrace::vector const& b) -> raytrace::vector {
                     using namespace raytrace;
                     using namespace geometry::operators;
                     raytrace::vector c;
                     c = a - b;
                     return c;
                 })
            .def("__getitem__", [](raytrace::vector& vec, size_t index) -> auto { return vec[index]; })
            .def("__setitem__",
                 [](raytrace::vector& vec, size_t index, raytrace::precision value) { vec[index] = value; })
            .def("__len__", [](raytrace::vector& vec) -> auto { return vec.dimensions; })
            .def("magnitude", &raytrace::vector::magnitude)
            .def("quadrance", &raytrace::vector::quadrance)
            .def("normalize", &raytrace::vector::normalize)
            .def("normalized", &raytrace::vector::normalized)
            .def("__repr__", [](raytrace::vector const& vec) -> auto {
                std::ostringstream oss;
                oss << vec;        // Use the C++ operator<<
                return oss.str();  // Return the resulting std::string
            });
        // define the dot and cross product methods which are not member functions and return floats() in python
        submodule.def("dot", [](raytrace::vector const& a, raytrace::vector const& b) -> precision {
            using namespace raytrace;
            using namespace geometry::operators;
            return dot(a, b);
        });
        submodule.def("cross", &raytrace::R3::cross);
        submodule.def("spread", [](raytrace::vector const& a, raytrace::vector const& b) -> precision {
            using namespace raytrace;
            using namespace geometry::operators;
            return spread(a, b);
        });

        // RAY Object
        py::class_<raytrace::ray> ray_class(submodule, "ray");
        ray_class.def(py::init<raytrace::point, raytrace::vector>())
            .def("position", &raytrace::ray::location)
            .def("direction", &raytrace::ray::direction)
            .def("distance_along", &raytrace::ray::distance_along)
            .def("closest", &raytrace::ray::closest)
            .def("__repr__", [](raytrace::ray const& r) -> auto {
                std::ostringstream oss;
                oss << r;          // Use the C++ operator<<
                return oss.str();  // Return the resulting std::string
            });

        // MEDIUM Object
        py::class_<raytrace::mediums::medium> medium_class(submodule, "medium");
        medium_class.def(py::init<>())
            .def("ambient", &raytrace::mediums::medium::ambient)
            .def("diffuse", &raytrace::mediums::medium::diffuse)
            .def("specular", &raytrace::mediums::medium::specular)
            .def("specular_tightness", &raytrace::mediums::medium::specular_tightness)
            .def("emissive", &raytrace::mediums::medium::emissive)
            .def("bounced", &raytrace::mediums::medium::bounced)
            .def("smoothness", &raytrace::mediums::medium::smoothness)
            .def("radiosity", &raytrace::mediums::medium::radiosity)
            .def("refractive_index", &raytrace::mediums::medium::refractive_index)
            .def("absorbance", &raytrace::mediums::medium::absorbance);

        // SPHERE Object
        py::class_<raytrace::objects::sphere>(submodule, "sphere")
            .def(py::init<raytrace::point, raytrace::precision>())
            .def("position", [](raytrace::objects::sphere& s) -> auto { return s.position(); })
            .def("move_by", &raytrace::objects::sphere::move_by)
            .def("get_material", [](raytrace::objects::sphere& s) -> auto { return s.material(); })
            .def("set_material",
                 [](raytrace::objects::sphere& s, raytrace::mediums::medium const& m) { s.material(&m); });
    }
}
