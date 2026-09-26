#include "raytrace/clouds/CloudLayer.hpp"

namespace raytrace {

namespace clouds {

CloudLayer::CloudLayer(point const& center, precision xhw, precision yhw, precision zhw)
    : m_center{center}
    , m_x_half_width{xhw}
    , m_y_half_width{yhw}
    , m_z_half_width{zhw}
    , m_extinction{0.25_p}
    , m_albedo{0.7_p}
    , m_seed{{0.67_p, 0.32_p, 0.81_p}}
    , m_octaves{6}
    , m_features_across{3.0_p}
    , m_horizontal_strength{0.0_p}
    , m_vertical{axis::Y}
    , m_density_callback{} {
    basal::exception::throw_if(basal::nearly_zero(xhw), __FILE__, __LINE__, "x half-width can't be zero");
    basal::exception::throw_if(basal::nearly_zero(yhw), __FILE__, __LINE__, "y half-width can't be zero");
    basal::exception::throw_if(basal::nearly_zero(zhw), __FILE__, __LINE__, "z half-width can't be zero");
}

point CloudLayer::center() const {
    return m_center;
}
precision CloudLayer::x_half_width() const {
    return m_x_half_width;
}
precision CloudLayer::y_half_width() const {
    return m_y_half_width;
}
precision CloudLayer::z_half_width() const {
    return m_z_half_width;
}

Bounds CloudLayer::bounds() const {
    point lo{m_center.x() - m_x_half_width, m_center.y() - m_y_half_width, m_center.z() - m_z_half_width};
    point hi{m_center.x() + m_x_half_width, m_center.y() + m_y_half_width, m_center.z() + m_z_half_width};
    return Bounds{lo, hi};
}

bool CloudLayer::contains(point const& world_point) const {
    return bounds().contained(world_point);
}

point CloudLayer::normalized(point const& world_point) const {
    return point{(world_point.x() - m_center.x()) / m_x_half_width, (world_point.y() - m_center.y()) / m_y_half_width,
                 (world_point.z() - m_center.z()) / m_z_half_width};
}

CloudLayer& CloudLayer::extinction(precision sigma) {
    m_extinction = sigma;
    return (*this);
}
precision CloudLayer::extinction() const {
    return m_extinction;
}

CloudLayer& CloudLayer::albedo(precision scattering) {
    m_albedo = std::clamp(scattering, 0.0_p, 1.0_p);
    return (*this);
}
precision CloudLayer::albedo() const {
    return m_albedo;
}

CloudLayer& CloudLayer::density_seed(noise::vector3 const& seed) {
    m_seed = seed;
    return (*this);
}

CloudLayer& CloudLayer::density_octaves(size_t octaves) {
    m_octaves = octaves;
    return (*this);
}

CloudLayer& CloudLayer::density_scale(precision features_across) {
    m_features_across = features_across;
    return (*this);
}

CloudLayer& CloudLayer::horizontal_strength(precision strength) {
    m_horizontal_strength = strength;
    return (*this);
}

CloudLayer& CloudLayer::density_callback(std::function<precision(point const&)> function) {
    m_density_callback = function;
    return (*this);
}

CloudLayer& CloudLayer::vertical_axis(axis vertical) {
    m_vertical = vertical;
    return (*this);
}

precision CloudLayer::vertical_profile(precision ny) const {
    ny = std::clamp(ny, -1.0_p, 1.0_p);
    // A raised cosine: 1 at the mid-plane, 0 at the top and bottom faces.
    return 0.5_p * (1.0_p + std::cos(ny * iso::pi));
}

precision CloudLayer::horizontal_envelope(precision nx, precision nz) const {
    if (basal::nearly_zero(m_horizontal_strength)) {
        return 1.0_p;
    }
    return std::exp(-m_horizontal_strength * ((nx * nx) + (nz * nz)));
}

precision CloudLayer::density(point const& world_point) const {
    point n = normalized(world_point);
    precision nx = std::clamp(n.x(), -1.0_p, 1.0_p);
    precision ny = std::clamp(n.y(), -1.0_p, 1.0_p);
    precision nz = std::clamp(n.z(), -1.0_p, 1.0_p);

    // Resolve which box axis is the vertical profile and which two are lateral.
    precision vertical = 0.0_p;
    precision lateral1 = 0.0_p;
    precision lateral2 = 0.0_p;
    switch (m_vertical) {
        case axis::Z:
            vertical = nz;
            lateral1 = nx;
            lateral2 = ny;
            break;
        case axis::X:
            vertical = nx;
            lateral1 = ny;
            lateral2 = nz;
            break;
        case axis::Y:  // [fall-through]
        default:
            vertical = ny;
            lateral1 = nx;
            lateral2 = nz;
            break;
    }

    precision value = 0.0_p;
    if (m_density_callback) {
        value = m_density_callback(world_point);
    } else {
        noise::point3 pnt{nx * m_features_across, ny * m_features_across, nz * m_features_across};
        precision fbm = noise::fractal_brownian(pnt, m_seed, m_octaves, 2.0_p, 0.5_p, 0.5_p, 1.0_p);
        value = fbm * vertical_profile(vertical) * horizontal_envelope(lateral1, lateral2);
    }
    return std::clamp(value, 0.0_p, 1.0_p);
}

}  // namespace clouds

}  // namespace raytrace