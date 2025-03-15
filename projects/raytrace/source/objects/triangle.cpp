#include "raytrace/objects/triangle.hpp"

#include <iostream>

namespace raytrace {
namespace objects {

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

triangle::triangle(R3::point const& A, R3::point const& B, R3::point const& C)
    : plane{R3::origin, R3::cross(A - B, C - B).normalized()}, m_points{} {
    // this defines the center of the given points which will be the translated center to the triangle
    point center = centroid(A, B, C);
    if constexpr (debug::triangle) {
        std::cout << "Triangle Center: " << center << " from A=" << A << " B=" << B << " C=" << C << std::endl;
    }
    // now relocate all the points so that the center is translated to where ever the points were defined as but each
    // point is not relative to the center
    vector delta = center - R3::origin;
    // adjust all the given points so that they are relative to the R3::origin
    m_points[0] = A - delta;
    m_points[1] = B - delta;
    m_points[2] = C - delta;
    if constexpr (debug::triangle) {
        std::cout << "\tMoved points to A=" << m_points[0] << " B=" << m_points[1] << " C=" << m_points[2] << std::endl;
    }
    // now the relocated center is moved
    position(center);
    if constexpr (debug::triangle) {
        std::cout << "\tMoved center to " << position() << std::endl;
    }
    // now find the farthest point from the origin of the triangle
    precision a = (m_points[0] - R3::origin).quadrance();
    precision b = (m_points[1] - R3::origin).quadrance();
    precision c = (m_points[2] - R3::origin).quadrance();
    m_radius2 = std::max(a, std::max(b, c));
}

bool triangle::is_contained(point const& object_point) const {
    precision r2 = (object_point - R3::origin).quadrance();
    // do we need to do a more in depth test?
    if (r2 < m_radius2) {
        static constexpr bool use_triple = true;
        // now determine if the point is in the triangle
        vector N = unormal();
        vector AB = m_points[1] - m_points[0];
        vector BC = m_points[2] - m_points[1];
        vector CA = m_points[0] - m_points[2];
        vector DA = object_point - m_points[0];
        vector DB = object_point - m_points[1];
        vector DC = object_point - m_points[2];
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
        if (basal::is_greater_than_or_equal_to_zero(NdDAxAB) and basal::is_greater_than_or_equal_to_zero(NdDBxBC)
            and basal::is_greater_than_or_equal_to_zero(NdDCxCA)) {
            return true;
        }
    }
    return false;
}

hits triangle::collisions_along(ray const& object_ray) const {
    auto collisions = plane::collisions_along(object_ray);
    hits hits;
    for (auto& h : collisions) {
        if (is_contained(as_point(h.intersect))) {  // this is in object space still
            // now we determine if the triangle normal is "facing" the ray or not
            precision projected_length = dot(h.normal, object_ray.direction());
            if (projected_length < 0.0_p) {
                hits.push_back(h);
            }
        }
    }
    return hits;
}

bool triangle::is_surface_point(point const& world_point) const {
    point object_point = reverse_transform(world_point);
    vector T = world_point - position();
    return basal::nearly_zero(dot(unormal(), T)) and is_contained(world_point);
}

void triangle::print(char const str[]) const {
    std::cout << str << " " << *this << std::endl;
}

std::ostream& operator<<(std::ostream& os, triangle const& tri) {
    os << "Triangle " << tri.points()[0] << "," << tri.points()[1] << "," << tri.points()[2] << " with centroid of "
       << tri.position() << std::endl;
    return os;
}

const std::array<point, raytrace::dimensions>& triangle::points() const {
    return m_points;
}

precision triangle::get_object_extent(void) const {
    return sqrt(m_radius2);
}

}  // namespace objects

geometry::plane as_plane(objects::triangle const& tri) {
    return tri;  // now a triangle is a special plane
}

}  // namespace raytrace
