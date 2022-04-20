#include "raytrace/mediums/medium.hpp"

namespace raytrace {
namespace mediums {
// Surface Constructor
medium::medium()
    : m_ambient_scale{mediums::ambient::none}
    , m_ambient{colors::black}
    , m_diffuse{colors::grey}
    , m_tightness{0.0f}
    , m_smoothness{mediums::smoothness::none}
    , m_transmissivity{0.0}
    , m_refractive_index{0.0}
    , m_reducing_map{nullptr} {
}

color medium::ambient(const raytrace::point& volumetric_point __attribute__((unused))) const {
    using namespace operators;
    return m_ambient * m_ambient_scale;
}

color medium::diffuse(const raytrace::point& volumetric_point __attribute__((unused))) const {
    return m_diffuse;
}

color medium::specular(const raytrace::point& volumetric_point, element_type scaling, const color& light_color) const {
    using namespace operators;
    // phong model?
    if (scaling > 0) {
        return std::pow(scaling, specular_tightness(volumetric_point)) * light_color;
    } else {
        return colors::black;
    }
}

element_type medium::specular_tightness(const raytrace::point& volumetric_point __attribute__((unused))) const {
    return m_tightness;
}

color medium::emissive(const raytrace::point& volumetric_point __attribute__((unused))) const {
    return colors::black;
}

color medium::bounced(const raytrace::point& volumetic_point __attribute__((unused)), const color& incoming) const {
    // just return incoming
    return incoming;
}

element_type medium::smoothness(const raytrace::point& volumetric_point __attribute__((unused))) const {
    return m_smoothness;
}

void medium::radiosity(const raytrace::point& volumetric_point __attribute__((unused)),
                       element_type refractive_index __attribute__((unused)),
                       const iso::radians& incident_angle __attribute__((unused)),
                       const iso::radians& transmitted_angle __attribute__((unused)), element_type& emitted,
                       element_type& reflected, element_type& transmitted) const {
    emitted = 0;
    reflected = 1.0 - m_transmissivity;
    transmitted = m_transmissivity;
}

element_type medium::refractive_index(const raytrace::point& volumetric_point __attribute__((unused))) const {
    return m_refractive_index;
}

color medium::absorbance(element_type /*distance*/, const color& given_color) const {
    return given_color;
}

mapping::reducer medium::mapper() const {
    return m_reducing_map;
}

void medium::mapper(mapping::reducer mapper) {
    m_reducing_map = mapper;
}

raytrace::vector medium::perturbation(const raytrace::point&) const {
    return geometry::R3::null;
}

}  // namespace mediums
}  // namespace raytrace
