#include "geometry/extra_math.hpp"

#include <cassert>

namespace geometry {

precision dot(R3::vector const& a, R3::point const& b) {
    basal::exception::throw_unless(a.dimensions == b.dimensions, __FILE__, __LINE__, "");
    return dot(a, b - R3::origin);
}

R3::point centroid(std::vector<R3::point> const& points) {
    R3::point D;
    for (size_t j = 0; j < points.size(); j++) {
        auto& p = points[j];
        for (size_t i = 0; i < D.dimensions; i++) {
            D[i] += p[i];
        }
    }
    for (size_t i = 0; i < D.dimensions; i++) {
        D[i] /= static_cast<precision>(points.size());
    }
    return D;
}

R3::point centroid(R3::point const& A, R3::point const& B, R3::point const& C) {
    R3::point D;
    for (size_t i = 0; i < D.dimensions; i++) {
        D[i] = (A[i] + B[i] + C[i]) / 3;
    }
    return D;
}

R3::point centroid(R3::point const& A, R3::point const& B) {
    R3::point D;
    for (size_t i = 0; i < D.dimensions; i++) {
        D[i] = (A[i] + B[i]) / 2;
    }
    return D;
}

std::pair<R3::point, R3::point> closest_points_from_skew_lines(R3::line const& first, R3::line const& second) {
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
    // @see https://en.wikipedia.org/wiki/Rotation_matrix
    precision i = axis[0];
    precision j = axis[1];
    precision k = axis[2];
    precision const o = 1.0_p;
    precision cos_t = std::cos(theta.value);
    precision sin_t = std::sin(theta.value);
    precision one_cos_t = o - cos_t;
    precision a = (i * i * one_cos_t) + (o * cos_t);
    precision b = (i * j * one_cos_t) - (k * sin_t);
    precision c = (i * k * one_cos_t) + (j * sin_t);

    precision d = (i * j * one_cos_t) + (k * sin_t);
    precision e = (j * j * one_cos_t) + (o * cos_t);
    precision f = (j * k * one_cos_t) - (i * sin_t);

    precision g = (i * k * one_cos_t) - (j * sin_t);
    precision h = (j * k * one_cos_t) + (i * sin_t);
    precision m = (k * k * one_cos_t) + (o * cos_t);
    return linalg::matrix{{{a, b, c}, {d, e, f}, {g, h, m}}};
}

linalg::matrix rotation(iso::radians const& yaw, iso::radians const& pitch, iso::radians const& roll) {
    // @see https://en.wikipedia.org/wiki/Rotation_matrix
    precision cos_yaw = std::cos(yaw.value);
    precision sin_yaw = std::sin(yaw.value);
    precision cos_pitch = std::cos(pitch.value);
    precision sin_pitch = std::sin(pitch.value);
    precision cos_roll = std::cos(roll.value);
    precision sin_roll = std::sin(roll.value);
    precision a = cos_yaw * cos_pitch;
    precision b = (cos_yaw * sin_pitch * sin_roll) - (sin_yaw * cos_roll);
    precision c = (cos_yaw * sin_pitch * cos_roll) + (sin_yaw * sin_roll);
    precision d = sin_yaw * cos_pitch;
    precision e = (sin_yaw * sin_pitch * sin_roll) + (cos_yaw * cos_roll);
    precision f = (sin_yaw * sin_pitch * cos_roll) - (cos_yaw * sin_roll);
    precision g = -sin_pitch;
    precision h = cos_pitch * sin_roll;
    precision i = cos_pitch * cos_roll;
    return linalg::matrix{{{a, b, c}, {d, e, f}, {g, h, i}}};
}

bool contained_within_aabb(R3::point const& P, R3::point const& min, R3::point const& max) {
    return (min.x - basal::epsilon < P.x and P.x < max.x + basal::epsilon)
           and (min.y - basal::epsilon < P.y and P.y < max.y + basal::epsilon)
           and (min.z - basal::epsilon < P.z and P.z < max.z + basal::epsilon);
}

}  // namespace geometry
