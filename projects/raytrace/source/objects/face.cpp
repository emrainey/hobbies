#include <raytrace/objects/face.hpp>

namespace raytrace {
namespace objects {

face::face(point const& A, point const& B, point const& C) : triangle(A, B, C) {
    m_texture_coords[0] = image::point{1, 0};
    m_texture_coords[1] = image::point{0, 0};
    m_texture_coords[2] = image::point{0, 1};
    m_normals[0] = m_normal;
    m_normals[1] = m_normal;
    m_normals[2] = m_normal;
}

face::face(point const& A, point const& B, point const& C, image::point const& a, image::point const& b, image::point const& c) : triangle(A, B, C) {
    m_texture_coords[0] = a;
    m_texture_coords[1] = b;
    m_texture_coords[2] = c;
    m_normals[0] = m_normal;
    m_normals[1] = m_normal;
    m_normals[2] = m_normal;
}

face::face(point const& A, point const& B, point const& C, image::point const& a, image::point const& b, image::point const& c, vector const& na, vector const& nb, vector const& nc) : triangle(A, B, C) {
    m_texture_coords[0] = a;
    m_texture_coords[1] = b;
    m_texture_coords[2] = c;
    m_normals[0] = na;
    m_normals[1] = nb;
    m_normals[2] = nc;
}

// /// @copydoc raytrace::object::intersect
// hit face::intersect(ray const& world_ray) const {
//     return triangle::intersect(world_ray);
// }

/// @copydoc basal::printable::print
void face::print(char const str[]) const {
    triangle::print(str);
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
    image::vector ab = (m_texture_coords[0] - m_texture_coords[1]); // the 2d vector along AB
    image::vector cb = (m_texture_coords[2] - m_texture_coords[1]); // the 2d vector along CB
    // scale the vectors by the normalized values
    image::vector uv_ab = a * ab;
    image::vector uv_db = b * cb;
    image::point uv = m_texture_coords[1] + (uv_ab + uv_db);
    return uv;
}

} // namespace objects
}  // namespace raytrace
