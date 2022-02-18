#include <cassert>
#include "geometry/extra_math.hpp"

namespace geometry {

element_type dot(const R3::vector &a, const R3::point &b) {
    basal::exception::throw_unless(a.dimensions == b.dimensions, __FILE__, __LINE__, "");
    element_type d = 0.0;
    for (size_t i = 0; i < b.dimensions; i++) {
        d += a[i] * b[i];
    }
    return d;
}

point centroid(const point& A, const point& B, const point& C) {
    basal::exception::throw_unless(A.dimensions == B.dimensions and B.dimensions == C.dimensions, __FILE__, __LINE__, "Must have same dimensionality");
    point D(B.dimensions);
    for (size_t i = 0; i < D.dimensions; i++) {
        D[i] = (A[i] + B[i] + C[i]) / 3;
    }
    return point(D);
}

std::pair<point, point> closest_points_from_skew_lines(const R3::line& first, const R3::line& second) {
    // get a normal which is orthagonal to both lines.
    R3::vector N1 = R3::cross(first.direction(), second.direction());
    // create another vector which is orthagonal to the first normal
    R3::vector N2 = R3::cross(second.direction(), N1);
    // we can make a plane at the second's location and find the intersection of the first with that plane
    plane sP(N2, second.position());
    // the intersection will be point A
    intersection isPf = intersects(sP, first);
    assert(get_type(isPf) == IntersectionType::Point);
    // closest point on first to second
    R3::point A(as_point(isPf));
    R3::vector N3 = R3::cross(first.direction(), N1);
    plane fP(N3, first.position());
    intersection ifPs = intersects(fP, second);
    assert(get_type(ifPs) == IntersectionType::Point);
    R3::point B(as_point(ifPs));
    return std::make_pair(A, B);
}

R2::point cartesian_to_polar(const R2::point& cartesian_point) {
    element_type x = cartesian_point.x;
    element_type y = cartesian_point.y;
    element_type r = sqrt(x*x + y*y);
    element_type theta = std::atan2(y, x);
    return R2::point(r, theta);
}

R2::point polar_to_cartesian(const R2::point& polar) {
    element_type r = polar.x;
    element_type theta = polar.y;
    element_type x = r * std::sin(theta);
    element_type y = r * std::cos(theta);
    return R2::point(x, y);
}

R3::point cartesian_to_spherical(const R3::point& cartesian_point) {
    element_type x = cartesian_point.x;
    element_type y = cartesian_point.y;
    element_type z = cartesian_point.z;
    element_type r = sqrt(x*x + y*y + z*z);
    element_type theta = atan2(y, x);
    element_type phi = atan2(sqrt(x*x + y*y), z);
    return R3::point(r, theta, phi);
}


R3::point spherical_to_cartesian(const R3::point& spherical_point) {
    element_type radius = spherical_point.x;
    element_type theta = spherical_point.y;
    element_type phi = spherical_point.z;
    element_type x = radius * std::sin(phi) * std::cos(theta);
    element_type y = radius * std::sin(phi) * std::sin(theta);
    element_type z = radius * std::cos(phi);
    return R3::point(x, y, z);
}

linalg::matrix rotation(const R3::vector& axis, const iso::radians theta) {
    element_type a = axis[0];
    element_type b = axis[1];
    element_type c = axis[2];
    element_type o = 1.0;
    element_type cos_t = std::sin(theta.value + iso::pi/2.0);
    element_type sin_t = std::sin(theta.value);
    element_type one_cos_t = o - cos_t;
    linalg::matrix r{{{(a*a*one_cos_t)+(o*cos_t),(a*b*one_cos_t)-(c*sin_t),(a*c*one_cos_t)+(b*sin_t)},
                      {(a*b*one_cos_t)+(c*sin_t),(b*b*one_cos_t)+(o*cos_t),(b*c*one_cos_t)-(a*sin_t)},
                      {(a*c*one_cos_t)-(b*sin_t),(b*c*one_cos_t)+(a*sin_t),(c*c*one_cos_t)+(o*cos_t)}}};
    return r;
}

bool contained_within_aabb(const R3::point& P, const R3::point& min, const R3::point& max) {
    return (min.x - basal::epsilon < P.x and P.x < max.x + basal::epsilon) and
           (min.y - basal::epsilon < P.y and P.y < max.y + basal::epsilon) and
           (min.z - basal::epsilon < P.z and P.z < max.z + basal::epsilon);
}

} // namespace geometry
