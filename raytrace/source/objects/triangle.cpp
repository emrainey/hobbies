#include <iostream>
#include "raytrace/objects/triangle.hpp"

namespace raytrace {
namespace objects {

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

triangle::triangle(const R3::point& A, const R3::point& B, const R3::point& C)
    : plane(centroid(A, B, C), R3::cross(A-B, C-B).normalized(), 1.0)
    , m_points()
    {
    m_points[0] = A;
    m_points[1] = B;
    m_points[2] = C;
    element_type a = (position() - A).magnitude();
    element_type b = (position() - B).magnitude();
    element_type c = (position() - C).magnitude();
    m_radius2 = std::max(a, std::max(b, c));
    m_radius2 *= m_radius2;
}

intersection triangle::intersect(const ray& world_ray) const {
    intersection inter = plane::intersect(world_ray);
    if (get_type(inter) == IntersectionType::Point) {
        point D = as_point(inter);
        element_type r2 = (position() - D).quadrance();
        // do we need to do a more indepth test?
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
            element_type NdDAxAB;
            element_type NdDBxBC;
            element_type NdDCxCA;
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
                element_type projected_length = dot(N, world_ray.direction());
                if (projected_length < 0) {
                    // pointing at each other, it's a hit
                    statistics::get().intersections_with_objects++;
                    return intersection(inter);
                }
            }
        }
        // didn't work out...
        statistics::get().intersections_with_objects--;
    }
    return intersection();
}

void triangle::print(const char str[]) const {
    std::cout << str << " Triangle @" << this << " " << m_points[0] << "," << m_points[1] << "," << m_points[2] << std::endl;
}

const std::array<point, 3>& triangle::points() const {
    return m_points;
}

} // namespace objects

geometry::plane as_plane(const objects::triangle& tri) {
    return tri; // now a triangle is a special plane
}

} // namespace raytrace
