#include "basal/exception.hpp"
#include "geometry/plane.hpp"
#include "raytrace/mapping.hpp"

namespace raytrace {

namespace mapping {

geometry::R2::point spherical(geometry::R3::point const& s) {
    basal::exception::throw_if(s == geometry::R3::origin, __FILE__, __LINE__, "Can't map the origin to a sphere");
    geometry::R3::vector q = s - geometry::R3::origin;
    geometry::R3::point p = geometry::R3::origin + q.normalized();
    precision u = (std::atan2(p.y, p.x) + iso::pi) / iso::tau;  // 0-theta-2pi
    u = (u < 0 ? 1.0 + u : u);
    precision v = std::acos(p.z) / iso::pi;  // 0-phi-pi
    return geometry::R2::point(u, v);
}

geometry::R2::point spherical(geometry::R3::vector const& q) {
    basal::exception::throw_if(q == geometry::R3::null, __FILE__, __LINE__, "Can't map the origin to a sphere");
    geometry::R3::point p = geometry::R3::origin + q.normalized();
    precision u = (std::atan2(p.y, p.x) + iso::pi) / iso::tau;  // 0-theta-2pi
    u = (u < 0 ? 1.0 + u : u);
    precision v = std::acos(p.z) / iso::pi;  // 0-phi-pi
    return geometry::R2::point(u, v);
}

geometry::R2::point cylindrical(precision scale, geometry::R3::point const& p) {
    geometry::R2::point cartesian(p.x, p.y);
    basal::exception::throw_if(cartesian == geometry::R2::origin, __FILE__, __LINE__,
                               "Can't map the origin to a cylinder");
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

geometry::R2::point toroidal(precision r1, geometry::R3::point const& world_point) {
    using namespace geometry;
    // create a toriodal mapping from a given inner radius and a point in 3 space
    // construct the projected vector from the origin to the point in 3d
    precision x = world_point.x;
    precision y = world_point.y;
    basal::exception::throw_if(basal::nearly_zero(x) and basal::nearly_zero(y), __FILE__, __LINE__,
                               "Can't map the origin to a torus");
    precision z = world_point.z;
    // angle around "Z"
    precision theta = std::atan2(y, x);
    precision r_xy = std::sqrt((x * x) + (y * y));
    precision d_rz = r_xy - r1;
    // angle around the tube
    precision phi = std::atan2(z, d_rz);
    // convert to 0-1 range
    precision u = (theta + iso::pi) / iso::tau;
    precision v = (phi + iso::pi) / iso::tau;
    return R2::point(u, v);
}

geometry::R2::point planar_polar(raytrace::vector const& N, raytrace::vector const& X, raytrace::point const& C,
                                 geometry::R3::point const& world_point) {
    using namespace geometry;
    using namespace iso::operators;
    basal::exception::throw_unless(basal::nearly_zero(dot(X, C)), __FILE__, __LINE__,
                                   "Can't map the origin to a plane");
    // create a planar polar mapping given a Normal ray and the "X" unit point
    plane pl0{C, N};
    // find the vector from the center to the point
    R3::vector v = world_point - C;
    // find the distance from the plane to the world point
    precision d = pl0.distance(world_point);
    // find the point on the plane
    R3::point P = world_point - (N * d);
    // find the in plane vector from X to P
    R3::vector Pr = P - C;
    // find the magnitude of the vector
    precision r = Pr.magnitude();
    // find the angle from the plane's x axis to the vector in the plane, but it only measures from 0 to pi
    precision theta0 = angle(X, Pr).value;
    // find the angle between the computed Y basis and the normal. N is considered to be like the +Z axis
    R3::vector Y = cross(N, X);
    // compute the angle between the Y basis and the vector in the plane to help determine the turns around the Normal
    // axis
    precision theta1 = angle(Y, Pr).value;
    // printf("Theta0: %f, Theta1: %f\n", theta0, theta1);
    precision theta = theta0;
    // if the theta1 is > pi/2 then we need to consider the theta0 angle as a negative angle.
    if (theta1 > iso::pi / 2) {
        theta = (iso::pi - theta0) + iso::pi;
    }

    // now turn it into a 0-1 range
    theta /= iso::tau;
    // printf("Theta: %f\n", theta);
    return R2::point{r, theta};
}

geometry::R3::point golden_ratio_mapper(size_t const numerator, size_t const denominator) {
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
