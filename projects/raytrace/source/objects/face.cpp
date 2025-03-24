#include <raytrace/objects/face.hpp>

namespace raytrace {
namespace objects {

face::face(point const& A, point const& B, point const& C)
    : triangle(A, B, C)
    , m_texture_coords{image::point{1, 0}, image::point{0, 0}, image::point{0, 1}}
    , m_normals{m_normal, m_normal, m_normal} {
    m_type = Type::Face;
}

face::face(point const& A, point const& B, point const& C, image::point const& a, image::point const& b,
           image::point const& c)
    : triangle(A, B, C), m_texture_coords{a, b, c}, m_normals{m_normal, m_normal, m_normal} {
}

face::face(point const& A, point const& B, point const& C, image::point const& a, image::point const& b,
           image::point const& c, vector const& na, vector const& nb, vector const& nc)
    : triangle(A, B, C), m_texture_coords{a, b, c}, m_normals{na, nb, nc} {
}

hits face::collisions_along(ray const& object_ray) const {
    return triangle::collisions_along(object_ray);
}

void face::print(std::ostream& os, char const str[]) const {
    os << str << " Face @" << this << " " << position() << " Normal " << unormal() << " A=" << m_points[0]
       << " B=" << m_points[1] << " C=" << m_points[2] << std::endl;
}

bool face::is_surface_point(point const& world_point) const {
    return triangle::is_surface_point(world_point);
}

precision face::get_object_extent(void) const {
    return triangle::get_object_extent();
}

vector face::normal_(point const&) const {
    vector N = m_normals[0] + m_normals[1] + m_normals[2];
    return N.normalize();
}

image::point face::map(point const& object_surface_point) const {
    // interpolate between the three texture coords based on the point
    vector AB = m_points[0] - m_points[1];
    vector CB = m_points[2] - m_points[1];
    vector DB = object_surface_point - m_points[1];
    // create the normalized scalars
    precision a = dot(AB, DB) / AB.magnitude();
    precision b = dot(CB, DB) / CB.magnitude();
    // create the 2d vector along ab, cb in UV space
    image::vector ab = (m_texture_coords[0] - m_texture_coords[1]);  // the 2d vector along AB
    image::vector cb = (m_texture_coords[2] - m_texture_coords[1]);  // the 2d vector along CB
    // scale the vectors by the normalized values
    image::vector uv_ab = a * ab;
    image::vector uv_db = b * cb;
    image::point uv = m_texture_coords[1] + (uv_ab + uv_db);
    return uv;
}

}  // namespace objects
}  // namespace raytrace
