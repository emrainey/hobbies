#include "raytrace/mediums/transparent.hpp"

#include "raytrace/laws.hpp"

namespace raytrace {

namespace mediums {

transparent::transparent(precision eta, color const& extinction, color const& diffuse)
    : dielectric{}, m_extinction{extinction} {
    m_diffuse = diffuse;
    m_smoothness = mediums::smoothness::perfect_mirror;  // no "surface colors"
    m_refractive_index = eta;
}

void transparent::radiosity(raytrace::point const& volumetric_point __attribute__((unused)), precision refractive_index,
                            iso::radians const& incident_angle, iso::radians const& transmitted_angle,
                            precision& emitted, precision& reflected, precision& transmitted) const {
    emitted = 0.0_p;
    if (basal::is_nan(transmitted_angle.value)) {
        reflected = 1.0_p;
    } else {
        reflected = laws::fresnel(refractive_index, m_refractive_index, incident_angle, transmitted_angle);
    }
    transmitted = 1.0_p - reflected;
}

color transparent::absorbance(precision distance, color const& given_color) const {
    using namespace operators;
    if (std::isinf(distance)) {
        // At infinite distance (background rays), return the background color directly.
        // For extinction=0 (vacuum), the Beer-Lambert formula would produce NaN (0 * inf).
        // For extinction>0 (haze), T→0 would blend toward m_diffuse, but the world's
        // background() already provides the correct sky color.
        return given_color;
    }
    color T(std::exp(-m_extinction.red() * distance), std::exp(-m_extinction.green() * distance),
            std::exp(-m_extinction.blue() * distance));
    color result(given_color.red() * T.red(), given_color.green() * T.green(), given_color.blue() * T.blue());
    // Only add inscatter when there is actual light to scatter (prevents self-lit appearance)
    if (given_color.intensity() > basal::epsilon) {
        result += color(m_diffuse.red() * (1.0_p - T.red()), m_diffuse.green() * (1.0_p - T.green()),
                        m_diffuse.blue() * (1.0_p - T.blue()));
    }
    return result;
}

}  // namespace mediums

}  // namespace raytrace
