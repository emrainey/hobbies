#include "raytrace/objects/polygon.hpp"

#include <iostream>

namespace raytrace {
namespace objects {

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

template <size_t N>
polygon<N>::polygon(std::initializer_list<R3::point> points) : plane{R3::origin, R3::basis::Z}, m_points{} {
    std::vector<R3::point> point_vector{points};
    basal::exception::throw_unless(N == point_vector.size(), __FILE__, __LINE__,
                                   "The number of points must be equal to the number of points in the polygon");
    m_normal = R3::cross(point_vector[0] - point_vector[1], point_vector[2] - point_vector[1]).normalized();
    eq.a = m_normal[0];
    eq.b = m_normal[1];
    eq.c = m_normal[2];
    // this defines the center of the given points which will be the translated center to the polygon
    point center = centroid(points);
    m_type = Type::Polygon;
    m_has_definite_volume = false;  // a polygon is a bounded planar surface
    if constexpr (debug::polygon) {
        std::cout << "Polygon Center: " << center << std::endl;
    }
    // now relocate all the points so that the center is translated to where ever the points were defined as but each
    // point is not relative to the center
    vector delta = center - R3::origin;
    // adjust all the given points so that they are relative to the R3::origin
    m_points[0] = point_vector[0] - delta;
    m_points[1] = point_vector[1] - delta;
    m_points[2] = point_vector[2] - delta;
    if constexpr (debug::polygon) {
        std::cout << "\tMoved points to A=" << m_points[0] << " B=" << m_points[1] << " C=" << m_points[2] << std::endl;
    }
    // now the relocated center is moved
    position(center);
    if constexpr (debug::polygon) {
        std::cout << "\tMoved center to " << position() << std::endl;
    }
    // now find the farthest point from the origin of the polygon
    precision a = (m_points[0] - R3::origin).quadrance();
    precision b = (m_points[1] - R3::origin).quadrance();
    precision c = (m_points[2] - R3::origin).quadrance();
    m_radius2 = std::max(a, std::max(b, c));
}

template <size_t N>
polygon<N>::polygon(std::array<R3::point, N> const& points)
    : plane{R3::origin, R3::cross(points[0] - points[1], points[2] - points[1]).normalized()}, m_points{points} {
    // this defines the center of the given points which will be the translated center to the polygon
    point center = centroid(points);
    m_type = Type::Polygon;
    m_has_definite_volume = false;  // a polygon is a bounded planar surface
    if constexpr (debug::polygon) {
        std::cout << "Polygon Center: " << center << std::endl;
    }
    // now relocate all the points so that the center is translated to where ever the points were defined as but each
    // point is not relative to the center
    vector delta = center - R3::origin;
    // adjust all the given points so that they are relative to the R3::origin
    m_points[0] -= delta;
    m_points[1] -= delta;
    m_points[2] -= delta;
    if constexpr (debug::polygon) {
        std::cout << "\tMoved points to A=" << m_points[0] << " B=" << m_points[1] << " C=" << m_points[2] << std::endl;
    }
    // now the relocated center is moved
    position(center);
    if constexpr (debug::polygon) {
        std::cout << "\tMoved center to " << position() << std::endl;
    }
    // now find the farthest point from the origin of the polygon
    precision a = (m_points[0] - R3::origin).quadrance();
    precision b = (m_points[1] - R3::origin).quadrance();
    precision c = (m_points[2] - R3::origin).quadrance();
    m_radius2 = std::max(a, std::max(b, c));
}

template <size_t N>
bool polygon<N>::is_contained(point const& object_point) const {
    precision r2 = (object_point - R3::origin).quadrance();
    // do we need to do a more in depth test?
    if (r2 < m_radius2) {
        // now determine if the point is in the polygon
        std::array<precision, N> proj;
        for (size_t i = 0; i < N; ++i) {
            auto edge = m_points[(i + 1) % N] - m_points[i];
            auto test = object_point - m_points[i];
            proj[i] = R3::triple(m_normal, test, edge);
        }
        // now, we must find any proj that are negative to determine if the point is outside the polygon
        for (size_t i = 0; i < N; ++i) {
            if (proj[i] < 0.0_p) {  // FIXME potentially use a basal:: function?
                return false;
            }
        }
        return true;
    }
    // the point is outside farthest point of the polygon from the center
    return false;
}

template <size_t N>
hits polygon<N>::collisions_along(ray const& object_ray) const {
    auto collisions = plane::collisions_along(object_ray);
    hits hits;
    for (auto& h : collisions) {
        if (is_contained(as_point(h.intersect))) {  // this is in object space still
            // now we determine if the polygon normal is "facing" the ray or not
            precision projected_length = dot(h.normal, object_ray.direction());
            if (projected_length < 0.0_p) {
                hits.push_back(h);
            }
        }
    }
    return hits;
}

template <size_t N>
bool polygon<N>::is_surface_point(point const& world_point) const {
    point object_point = reverse_transform(world_point);
    vector T = world_point - position();
    return basal::nearly_zero(dot(unormal(), T)) and is_contained(world_point);
}

template <size_t N>
void polygon<N>::print(std::ostream& os, char const name[]) const {
    os << "Triangle " << this << " " << name << " " << points()[0] << "," << points()[1] << "," << points()[2]
       << " with centroid of " << position() << std::endl;
}

template <size_t N>
std::array<point, N> const& polygon<N>::points() const {
    return m_points;
}

template <size_t N>
precision polygon<N>::get_object_extent(void) const {
    return sqrt(m_radius2);
}

// EXPLICIT INSTANTIATIONS
template class polygon<3U>;
template class polygon<4U>;
template class polygon<5U>;
template class polygon<6U>;
template class polygon<8U>;

}  // namespace objects

template <size_t N>
geometry::plane as_plane(objects::polygon<N> const& poly) {
    return poly;  // now a polygon is a special plane
}

// EXPLICIT INSTANTIATIONS
template geometry::plane as_plane<3>(objects::polygon<3> const& poly);
template geometry::plane as_plane<4>(objects::polygon<4> const& poly);
template geometry::plane as_plane<5>(objects::polygon<5> const& poly);
template geometry::plane as_plane<6>(objects::polygon<6> const& poly);
template geometry::plane as_plane<8>(objects::polygon<8> const& poly);

}  // namespace raytrace
