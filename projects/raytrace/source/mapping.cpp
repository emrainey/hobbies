#include "raytrace/mapping.hpp"

namespace raytrace {

namespace mapping {

geometry::R2::point spherical(geometry::R3::point const& s) {
    geometry::R3::vector q = s - geometry::R3::origin;
    geometry::R3::point p = geometry::R3::origin + q.normalized();
    precision u = std::atan2(p.y, p.x) / iso::tau;  // 0-theta-2pi
    u = (u < 0 ? 1.0 + u : u);
    precision v = std::acos(p.z) / iso::pi;  // 0-phi-pi
    return geometry::R2::point(u, v);
}

geometry::R2::point spherical(geometry::R3::vector const& q) {
    geometry::R3::point p = geometry::R3::origin + q.normalized();
    precision u = std::atan2(p.y, p.x) / iso::tau;  // 0-theta-2pi
    u = (u < 0 ? 1.0 + u : u);
    precision v = std::acos(p.z) / iso::pi;  // 0-phi-pi
    return geometry::R2::point(u, v);
}

geometry::R2::point cylindrical(precision scale, geometry::R3::point const& p) {
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
                             geometry::R3::point const& p __attribute__((unused))) {
    return geometry::R2::origin;
}

geometry::R2::point planar_polar(raytrace::ray const& N __attribute__((unused)),
                                 raytrace::point const& X __attribute__((unused)),
                                 geometry::R3::point const& p __attribute__((unused))) {
    return geometry::R2::origin;
}

geometry::R3::point
golden_ratio_mapper(size_t const numerator, size_t const denominator) {
    // derived from https://stackoverflow.com/questions/9600801/evenly-distributing-n-points-on-a-sphere

    // this part can be computed once and applied again and again
    static precision const golden_phi_radians = iso::pi * (1.0 + std::sqrt(5));
    precision const value = ((precision)numerator + 0.5) / (precision)denominator;
    precision const phi = std::acos(1.0 - (2.0 * value));
    precision const theta = golden_phi_radians * numerator;

    return geometry::R3::point(std::cos(theta) * std::sin(phi), std::sin(theta) * std::sin(phi), std::cos(phi));
}

}  // namespace mapping

}  // namespace raytrace
