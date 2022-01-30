#include "raytrace/medium.hpp"

namespace raytrace {

// Surface Constructor
medium::medium()
    : m_ambient_scale(ambient::none)
    , m_ambient(colors::black)
    , m_diffuse(colors::grey)
    , m_tightness(0.0f)
    , m_smoothness(smoothness::none)
    , m_transmissivity(0.0)
    , m_refractive_index(0.0)
    , m_reducing_map(nullptr)
    {}

color medium::ambient(const raytrace::point& volumetric_point __attribute__((unused))) const {
    using namespace operators;
    return m_ambient * m_ambient_scale;
}

color medium::diffuse(const raytrace::point& volumetric_point __attribute__((unused))) const {
    return m_diffuse;
}

color medium::specular(const raytrace::point& volumetric_point, element_type scaling, const color& light_color) const {
    using namespace operators;
    // phong model
    return std::pow(scaling, specular_tightness(volumetric_point)) * light_color;
}

element_type medium::specular_tightness(const raytrace::point& volumetric_point __attribute__((unused))) const {
    return m_tightness;
}

element_type medium::smoothness(const raytrace::point& volumetric_point __attribute__((unused))) const {
    return m_smoothness;
}

void medium::radiosity(const raytrace::point& volumetric_point __attribute__((unused)),
                       element_type refractive_index __attribute__((unused)),
                       const iso::radians& incident_angle __attribute__((unused)),
                       const iso::radians& transmitted_angle __attribute__((unused)),
                       element_type& emitted,
                       element_type& reflected,
                       element_type& transmitted) const {
    emitted = 0;
    reflected = 1.0 - m_transmissivity;
    transmitted = m_transmissivity;
}

element_type medium::refractive_index(const raytrace::point& volumetric_point __attribute__((unused))) const {
    return m_refractive_index;
}

element_type medium::absorbance(element_type distance __attribute__((unused))) const {
    return 1.0;
}

mapping::reducer medium::mapper() const {
    return m_reducing_map;
}

void medium::mapper(mapping::reducer mapper) {
    m_reducing_map = mapper;
}

} // namespace raytrace
