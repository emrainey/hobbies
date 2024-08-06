#include "raytrace/objects/triangle.hpp"

#include <iostream>

namespace raytrace {
namespace objects {

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

triangle::triangle(R3::point const& A, R3::point const& B, R3::point const& C)
    : plane{centroid(A, B, C), R3::cross(A - B, C - B).normalized()}, m_points{} {
    m_points[0] = A;
    m_points[1] = B;
    m_points[2] = C;
    precision a = (position() - A).magnitude();
    precision b = (position() - B).magnitude();
    precision c = (position() - C).magnitude();
    m_radius2 = std::max(a, std::max(b, c));
    m_radius2 *= m_radius2;
}

bool triangle::is_contained(point const& D) const {
    precision r2 = (position() - D).quadrance();
    // do we need to do a more in depth test?
    if (r2 < m_radius2) {
        constexpr static bool use_triple = true;
        // now determine if the point is in the triangle
        vector N = normal(D);
        vector AB = m_points[1] - m_points[0];
        vector BC = m_points[2] - m_points[1];
        vector CA = m_points[0] - m_points[2];
        vector DA = D - m_points[0];
        vector DB = D - m_points[1];
        vector DC = D - m_points[2];
        precision NdDAxAB;
        precision NdDBxBC;
        precision NdDCxCA;
        if constexpr (not use_triple) {
            vector DAxAB = R3::cross(DA, AB);
            vector DBxBC = R3::cross(DB, BC);
            vector DCxCA = R3::cross(DC, CA);
            NdDAxAB = dot(N, DAxAB);
            NdDBxBC = dot(N, DBxBC);
            NdDCxCA = dot(N, DCxCA);
        } else {
            NdDAxAB = R3::triple(N, DA, AB);
            NdDBxBC = R3::triple(N, DB, BC);
            NdDCxCA = R3::triple(N, DC, CA);
        }
        if (NdDAxAB >= 0 and NdDBxBC >= 0 and NdDCxCA >= 0) {
            return true;
        }
    }
    return false;
}

intersection triangle::intersect(ray const& world_ray) const {
    intersection inter = plane::intersect(world_ray);
    if (get_type(inter) == IntersectionType::Point) {
        point D = as_point(inter);
        if (is_contained(D)) {
            precision projected_length = dot(geometry::plane::normal, world_ray.direction());
            if (projected_length < 0) {
                return intersection(inter);
            }
        }
    }
    return intersection();
}

bool triangle::is_surface_point(point const& world_point) const {
    point object_point = reverse_transform(world_point);
    vector T = object_point - position();
    return basal::nearly_zero(dot(geometry::plane::normal, T)) and is_contained(world_point);
}

void triangle::print(char const str[]) const {
    std::cout << str << " Triangle @" << this << " " << m_points[0] << "," << m_points[1] << "," << m_points[2]
              << std::endl;
}

const std::array<point, 3>& triangle::points() const {
    return m_points;
}

precision triangle::get_object_extent(void) const {
    // find the point farthest from origin
    precision d[] = {
        (m_points[0] - R3::origin).magnitude(),
        (m_points[1] - R3::origin).magnitude(),
        (m_points[2] - R3::origin).magnitude(),
    };
    return std::max(d[0], std::max(d[1], d[2]));
}

}  // namespace objects

geometry::plane as_plane(objects::triangle const& tri) {
    return tri;  // now a triangle is a special plane
}

}  // namespace raytrace
