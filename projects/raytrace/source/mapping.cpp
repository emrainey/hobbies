#include "raytrace/mapping.hpp"

namespace raytrace {

namespace mapping {

geometry::R2::point spherical(const geometry::R3::point& s) {
    geometry::R3::vector q = s - geometry::R3::origin;
    geometry::R3::point p = geometry::R3::origin + q.normalized();
    precision u = std::atan2(p.y, p.x) / iso::tau;  // 0-theta-2pi
    u = (u < 0 ? 1.0 + u : u);
    precision v = std::acos(p.z) / iso::pi;  // 0-phi-pi
    return geometry::R2::point(u, v);
}

geometry::R2::point spherical(const geometry::R3::vector& q) {
    geometry::R3::point p = geometry::R3::origin + q.normalized();
    precision u = std::atan2(p.y, p.x) / iso::tau;  // 0-theta-2pi
    u = (u < 0 ? 1.0 + u : u);
    precision v = std::acos(p.z) / iso::pi;  // 0-phi-pi
    return geometry::R2::point(u, v);
}

geometry::R2::point cylindrical(precision scale, const geometry::R3::point& p) {
    geometry::R2::point cartesian(p.x, p.y);
    geometry::R2::point polar = geometry::cartesian_to_polar(cartesian);
    // some range of z based in the half_height we want -h2 to map to zero and +h2 to 1.0
    precision u = (p.z / (-2.0 * scale)) + 0.5;
    // theta goes from +pi to -pi we want to map -pi to 1.0 and + pi to zero
    precision v = 0.0;
    if (polar.y >= 0) {
        v = 0.5 - (polar.y / (+2.0 * iso::pi));
    } else {
        v = 0.5 + (polar.y / (-2.0 * iso::pi));
    }
    return geometry::R2::point(u, v);
}

geometry::R2::point toroidal(precision r1 __attribute__((unused)),
                             const geometry::R3::point& p __attribute__((unused))) {
    return geometry::R2::origin;
}

geometry::R2::point planar_polar(const raytrace::ray& N __attribute__((unused)),
                                 const raytrace::point& X __attribute__((unused)),
                                 const geometry::R3::point& p __attribute__((unused))) {
    return geometry::R2::origin;
}

geometry::R3::point golden_ratio_mapper(const size_t numerator, const size_t denominator) {
    // derived from https://stackoverflow.com/questions/9600801/evenly-distributing-n-points-on-a-sphere

    // this part can be computed once and applied again and again
    static const precision golden_phi_radians = iso::pi * (1.0 + std::sqrt(5));
    const precision value = ((precision)numerator + 0.5) / (precision)denominator;
    const precision phi = std::acos(1.0 - (2.0 * value));
    const precision theta = golden_phi_radians * numerator;

    return geometry::R3::point(std::cos(theta) * std::sin(phi), std::sin(theta) * std::sin(phi), std::cos(phi));
}

}  // namespace mapping

}  // namespace raytrace
