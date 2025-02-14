#include "raytrace/objects/object.hpp"

namespace raytrace {
namespace objects {
/// The mechanism by which one object can overlap another and modify ray results with normals.
/// @see overlap::type
///
class overlap : public object {
public:
    /// The various types of supported overlaps
    enum class type : int {
        additive,     //!< Joins two objects together
        subtractive,  //!< Subtracts object B from A
        exclusive,    //!< Renders only what is not overlapping between both items (will need to be overlapped again to
                      //!< show this areas)
        inclusive,    //!< Render only what overlaps between both objects. (Useful for lenses)
    };

    /// Constructs an overlap between two objects
    overlap(object const& A, object const& B, type t);

    virtual ~overlap() = default;

    /// @copydoc raytrace::object::intersect
    // geometry::intersection intersect(ray const& world_ray) const override;
    /// @copydoc raytrace::object::collision_along
    hits collisions_along(ray const& object_ray) const override;
    /// @copydoc raytrace::object::map
    image::point map(point const& object_surface_point) const override;
    /// @copydoc basal::printable::print
    void print(char const str[]) const override;

    bool is_surface_point(point const& world_point) const override;
    precision get_object_extent(void) const override;
    bool is_along_infinite_extent(ray const& world_ray) const override;
    bool is_outside(point const& world_point) const override;

protected:
    vector normal_(point const& object_surface_point) const override;

    object const& m_A;
    object const& m_B;
    type const m_type;
    bool m_closed_two_hit_surfaces_;
    bool m_open_two_hit_surfaces_;
    bool m_open_one_hit_surfaces_;
};
}  // namespace objects
}  // namespace raytrace