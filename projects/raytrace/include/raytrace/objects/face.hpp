#pragma once

#include <array>
#include <raytrace/objects/triangle.hpp>

namespace raytrace {
namespace objects {

class face : public triangle {
public:
    face(point const& A, point const& B, point const& C);

    face(point const& A, point const& B, point const& C, image::point const& a, image::point const& b,
         image::point const& c);

    face(point const& A, point const& B, point const& C, image::point const& a, image::point const& b,
         image::point const& c, vector const& na, vector const& nb, vector const& nc);

    virtual ~face() = default;

    // ┌─────────────────────────┐
    // │raytrace::objects::object│
    // └─────────────────────────┘
    hits collisions_along(ray const& object_ray) const override;
    bool is_surface_point(point const& world_point) const override;
    precision get_object_extent(void) const override;
    image::point map(point const& object_surface_point) const override;
    void print(std::ostream& os, char const str[]) const override;

protected:
    vector normal_(point const& object_surface_point) const override;

protected:
    std::array<image::point, raytrace::dimensions> m_texture_coords;
    std::array<raytrace::vector, raytrace::dimensions> m_normals;
};

}  // namespace objects
}  // namespace raytrace
