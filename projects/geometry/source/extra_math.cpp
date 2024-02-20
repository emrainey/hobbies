#include "geometry/extra_math.hpp"

#include <cassert>

namespace geometry {

precision dot(R3::vector const& a, R3::point const& b) {
    basal::exception::throw_unless(a.dimensions == b.dimensions, __FILE__, __LINE__, "");
    precision d = 0.0;
    for (size_t i = 0; i < b.dimensions; i++) {
        d += a[i] * b[i];
    }
    return d;
}

point centroid(point const& A, point const& B, point const& C) {
    basal::exception::throw_unless(A.dimensions == B.dimensions and B.dimensions == C.dimensions, __FILE__, __LINE__,
                                   "Must have same dimensionality");
    point D{B.dimensions};
    for (size_t i = 0; i < D.dimensions; i++) {
        D[i] = (A[i] + B[i] + C[i]) / 3;
    }
    return point(D);
}

std::pair<point, point> closest_points_from_skew_lines(R3::line const& first, R3::line const& second) {
    // get a normal which is orthagonal to both lines.
    R3::vector N1 = R3::cross(first.direction(), second.direction());
    // create another vector which is orthagonal to the first normal
    R3::vector N2 = R3::cross(second.direction(), N1);
    // we can make a plane at the second's location and find the intersection of the first with that plane
    plane sP{N2, second.position()};
    // the intersection will be point A
    intersection isPf = intersects(sP, first);
    assert(get_type(isPf) == IntersectionType::Point);
    // closest point on first to second
    R3::point A{as_point(isPf)};
    R3::vector N3 = R3::cross(first.direction(), N1);
    plane fP{N3, first.position()};
    intersection ifPs = intersects(fP, second);
    assert(get_type(ifPs) == IntersectionType::Point);
    R3::point B{as_point(ifPs)};
    return std::make_pair(A, B);
}

R2::point cartesian_to_polar(R2::point const& cartesian_point) {
    precision x = cartesian_point.x;
    precision y = cartesian_point.y;
    precision r = sqrt(x * x + y * y);
    precision theta = std::atan2(y, x);
    return R2::point(r, theta);
}

R2::point polar_to_cartesian(R2::point const& polar) {
    precision r = polar.x;
    precision theta = polar.y;
    precision x = r * std::sin(theta);
    precision y = r * std::cos(theta);
    return R2::point(x, y);
}

R3::point cartesian_to_spherical(R3::point const& cartesian_point) {
    precision x = cartesian_point.x;
    precision y = cartesian_point.y;
    precision z = cartesian_point.z;
    precision r = sqrt(x * x + y * y + z * z);
    precision theta = atan2(y, x);
    precision phi = atan2(sqrt(x * x + y * y), z);
    return R3::point(r, theta, phi);
}

R3::point spherical_to_cartesian(R3::point const& spherical_point) {
    precision radius = spherical_point.x;
    precision theta = spherical_point.y;
    precision phi = spherical_point.z;
    precision x = radius * std::sin(phi) * std::cos(theta);
    precision y = radius * std::sin(phi) * std::sin(theta);
    precision z = radius * std::cos(phi);
    return R3::point(x, y, z);
}

linalg::matrix rotation(R3::vector const& axis, iso::radians const theta) {
    precision a = axis[0];
    precision b = axis[1];
    precision c = axis[2];
    precision o = 1.0;
    precision cos_t = std::sin(theta.value + iso::pi / 2.0);
    precision sin_t = std::sin(theta.value);
    precision one_cos_t = o - cos_t;
    linalg::matrix r{
        {{(a * a * one_cos_t) + (o * cos_t), (a * b * one_cos_t) - (c * sin_t), (a * c * one_cos_t) + (b * sin_t)},
         {(a * b * one_cos_t) + (c * sin_t), (b * b * one_cos_t) + (o * cos_t), (b * c * one_cos_t) - (a * sin_t)},
         {(a * c * one_cos_t) - (b * sin_t), (b * c * one_cos_t) + (a * sin_t), (c * c * one_cos_t) + (o * cos_t)}}};
    return r;
}

bool contained_within_aabb(R3::point const& P, R3::point const& min, R3::point const& max) {
    return (min.x - basal::epsilon < P.x and P.x < max.x + basal::epsilon)
           and (min.y - basal::epsilon < P.y and P.y < max.y + basal::epsilon)
           and (min.z - basal::epsilon < P.z and P.z < max.z + basal::epsilon);
}

}  // namespace geometry
