
#include <noise/noise.hpp>
#include "raytrace/mediums/random_noise.hpp"

namespace raytrace {

namespace mediums {

random_noise::random_noise()
    : opaque()
    {
    m_ambient = colors::white;
    m_ambient_scale = ambient::dim;
}

color random_noise::diffuse(const raytrace::point& volumetric_point) const {
    palette pal;
    image::point p(volumetric_point.x, volumetric_point.y);
    return functions::pseudo_random_noise(p, pal);
}

} // namespace mediums

} // namespace raytrace
