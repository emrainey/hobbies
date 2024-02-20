
#include "raytrace/mediums/random_noise.hpp"

#include <noise/noise.hpp>

namespace raytrace {

namespace mediums {

random_noise::random_noise() : opaque{} {
    m_ambient = colors::white;
    m_ambient_scale = mediums::ambient::dim;
}

color random_noise::diffuse(raytrace::point const& volumetric_point) const {
    palette pal;
    image::point p{volumetric_point.x, volumetric_point.y};
    return functions::pseudo_random_noise(p, pal);
}

}  // namespace mediums

}  // namespace raytrace
