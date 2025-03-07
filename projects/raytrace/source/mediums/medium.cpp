#include "raytrace/mediums/medium.hpp"

namespace raytrace {
namespace mediums {
// Surface Constructor
medium::medium()
    : m_ambient_scale{mediums::ambient::none},
      m_ambient{colors::black},
      m_diffuse{colors::grey},
      m_tightness{0.0_p},
      m_smoothness{mediums::smoothness::none},
      m_reflectivity{0.5}  // start semi gloss
      ,
      m_transmissivity{0.0},
      m_refractive_index{0.0},
      m_reducing_map{nullptr} {
}

color medium::ambient(raytrace::point const& volumetric_point __attribute__((unused))) const {
    using namespace operators;
    return m_ambient * m_ambient_scale;
}

color medium::diffuse(raytrace::point const& volumetric_point __attribute__((unused))) const {
    return m_diffuse;
}

color medium::specular(raytrace::point const& volumetric_point, precision scaling, color const& light_color) const {
    using namespace operators;
    // phong model?
    if (scaling > 0) {
        return std::pow(scaling, specular_tightness(volumetric_point)) * light_color;
    } else {
        return colors::black;
    }
}

precision medium::specular_tightness(raytrace::point const& volumetric_point __attribute__((unused))) const {
    return m_tightness;
}

color medium::emissive(raytrace::point const& volumetric_point __attribute__((unused))) const {
    return colors::black;
}

color medium::bounced(raytrace::point const& volumetric_point __attribute__((unused)), color const& incoming) const {
    // just return incoming
    return incoming;
}

precision medium::smoothness(raytrace::point const& volumetric_point __attribute__((unused))) const {
    return m_smoothness;
}

void medium::radiosity(raytrace::point const& volumetric_point __attribute__((unused)),
                       precision refractive_index __attribute__((unused)),
                       iso::radians const& incident_angle __attribute__((unused)),
                       iso::radians const& transmitted_angle __attribute__((unused)), precision& emitted,
                       precision& reflected, precision& transmitted) const {
    emitted = 0;  //@FIXME when adding luminescence, do it here.
    reflected = m_reflectivity;
    transmitted = 1.0 - m_reflectivity;
}

precision medium::refractive_index(raytrace::point const& volumetric_point __attribute__((unused))) const {
    return m_refractive_index;
}

color medium::absorbance(precision /*distance*/, color const& given_color) const {
    return given_color;
}

mapping::reducer medium::mapper() const {
    return m_reducing_map;
}

void medium::mapper(mapping::reducer mapper) {
    m_reducing_map = mapper;
}

raytrace::vector medium::perturbation(raytrace::point const&) const {
    return geometry::R3::null;
}

}  // namespace mediums
}  // namespace raytrace
