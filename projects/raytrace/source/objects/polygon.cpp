#include "raytrace/objects/polygon.hpp"

#include <iostream>

namespace raytrace {
namespace objects {

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

template <size_t N>
polygon<N>::polygon(std::initializer_list<R3::point> points)
    : object{R3::origin, 1, Type::Polygon, false}, m_points{}, m_radius2{std::numeric_limits<precision>::min()} {
    std::vector<R3::point> point_vector{points};
    basal::exception::throw_unless(N == point_vector.size(), __FILE__, __LINE__,
                                   "The number of points must be equal to the number of points in the polygon");
    // this defines the center of the given points which will be the translated center to the polygon
    point center = centroid(points);
    if constexpr (debug::polygon) {
        std::cout << "Polygon Center: " << center << std::endl;
    }
    // now relocate all the points so that the center is translated to where ever the points were defined as but each
    // point is now relative to the center
    vector delta = center - R3::origin;
    // adjust all the given points so that they are relative to the center so that they are effectively relative to the
    // R3::origin afterwards
    for (size_t i = 0; i < N; ++i) {
        m_points[i] = point_vector[i] - delta;
        if constexpr (debug::polygon) {
            std::cout << "\tMoved [" << i << "] " << point_vector[i] << " -> " << m_points[i] << std::endl;
        }
        auto r = (m_points[i] - R3::origin).quadrance();
        if (m_radius2 < r) {
            m_radius2 = r;
        }
    }
    // now the relocated center is moved
    position(center);
    if constexpr (debug::polygon) {
        std::cout << "\tMoved center to " << position() << std::endl;
    }
}

template <size_t N>
polygon<N>::polygon(std::array<R3::point, N> const& points)
    : object{R3::origin, 1, Type::Polygon, false}, m_points{points} {
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
    for (size_t i = 0; i < N; ++i) {
        m_points[i] = points[i] - delta;
        if constexpr (debug::polygon) {
            std::cout << "\tMoved [" << i << "] " << points[i] << " -> " << m_points[i] << std::endl;
        }
        auto r = (m_points[i] - R3::origin).quadrance();
        if (m_radius2 < r) {
            m_radius2 = r;
        }
    }
    // now the relocated center is moved
    position(center);
    if constexpr (debug::polygon) {
        std::cout << "\tMoved center to " << position() << std::endl;
    }
}

template <size_t N>
R3::vector polygon<N>::normal_(point const&) const {
    // cross the first 3 points to get the normal
    return R3::cross(m_points[0] - m_points[1], m_points[2] - m_points[1]).normalized();
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
            proj[i] = R3::triple(normal_(R3::origin), test, edge);
        }
        // now, we must find any proj that are negative to determine if the point is outside the polygon
        for (size_t i = 0; i < N; ++i) {
            if (proj[i] < 0.0_p) {
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
    hits ts;
    // is the ray parallel to the plane?
    // @note in object space, the center point is at the origin
    vector const n = normal_(R3::origin);  // the normal is the same for all points
    vector const& V = object_ray.direction();
    precision const proj = dot(V, n);
    // if so the projection is not zero they collide *somewhere*
    // could be positive or negative t. Don't check for proj < 0.0_p since we may be
    // concerned with colliding with the back-side of walls
    if (not basal::nearly_zero(proj)) {  // they collide *somewhere*
        point const& P = object_ray.location();
        // get the vector of the center to the ray initial
        vector const C = R3::origin - P;
        // t is the ratio of the projection of the arbitrary center vector divided by the projection ray
        precision const t = dot(C, n) / proj;
        // D is the point on the line t distance along
        point D = object_ray.distance_along(t);
        if (is_contained(D)) {  // this is in object space still
            // now we determine if the polygon normal is "facing" the ray or not
            precision projected_length = dot(n, object_ray.direction());
            if (projected_length < 0.0_p) {
                ts.emplace_back(intersection{D}, t, n, this);
            }
        }
    }
    return ts;
}

template <size_t N>
image::point polygon<N>::map(point const& object_surface_point) const {
    // in object space the origin is the center and the normal is computed from the first 3 points
    // the polygon could have *any* orientation!
    vector w = object_surface_point - R3::origin;
    if (w == R3::null) {
        return image::point{0, 0};
    }
    // the first 3 points form a basis (possibly not orthagonal) in 2D.
    vector i = m_points[0] - m_points[1];
    vector j = m_points[2] - m_points[1];
    vector k = cross(i, j);
    R3::axes a{position(), i, j, k};
    R3::point b = a.to_basis() * object_surface_point;
    R2::point uv{b.x(), b.y()};
    return uv;
}

template <size_t N>
bool polygon<N>::is_surface_point(point const& world_point) const {
    point object_point = reverse_transform(world_point);
    vector world_delta = world_point - position();
    if (world_delta == R3::null) {
        return true;
    }
    vector const& n = normal_(R3::origin);  // all the same normals
    vector const& V = world_delta;
    precision const proj = dot(V, n);
    return basal::nearly_zero(proj) and is_contained(object_point);
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

}  // namespace raytrace
