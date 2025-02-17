#include "raytrace/objects/object.hpp"

namespace raytrace {
namespace objects {
/// The mechanism by which one object can overlap another and modify ray results with normals.
/// @see overlap::type
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
    /// @param A The first object (when the type is subtractive, this is the center of the object)
    /// @param B The second object (when the type is subtractive, this is the subtractive object)
    /// @param t The type of overlap
    /// When the type is subtractive, the position of the overlap object is at the center of A.
    /// All other types of overlaps have their position at the centroid of A and B.
    overlap(object const& A, object const& B, type t);

    virtual ~overlap() = default;

    // ┌────────────────┐
    // │raytrace::object│
    // └────────────────┘
    hits collisions_along(ray const& object_ray) const override;
    image::point map(point const& object_surface_point) const override;
    void print(char const str[]) const override;
    bool is_surface_point(point const& world_point) const override;
    precision get_object_extent(void) const override;
    bool is_along_infinite_extent(ray const& world_ray) const override;
    bool is_outside(point const& world_point) const override;
    size_t max_collisions(void) const override;
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