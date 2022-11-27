#include "raytrace/objects/object.hpp"

namespace raytrace {
namespace objects {
/** The mechanism by which one object can overlap another and modify ray results with normals.
 * @see overlap::type
 */
class overlap : public object {
public:
    /** The various types of supported overlaps */
    enum class type : int {
        additive,     //!< Joins two objects together
        subtractive,  //!< Subtracts object B from A
        exclusive,    //!< Renders only what is not overlapping between both items (will need to be overlapped again to
                      //!< show this areas)
        inclusive,    //!< Render only what overlaps between both objects. (Useful for lenses)
    };

    /** Constructs an overlap between two objects */
    overlap(const object& A, const object& B, type t);

    virtual ~overlap() = default;

    /** @copydoc raytrace::object::normal */
    vector normal(const point& world_surface_point) const override;
    /** @copydoc raytrace::object::intersect */
    // geometry::intersection intersect(const ray& world_ray) const override;
    /** @copydoc raytrace::object::collision_along */
    hits collisions_along(const ray& object_ray) const override;
    /** @copydoc raytrace::object::map */
    image::point map(const point& object_surface_point) const override;
    /** @copydoc basal::printable::print */
    void print(const char str[]) const override;

    inline size_t max_collisions() const override {
        return std::max(m_A.max_collisions(), m_B.max_collisions());
    }

    inline bool is_closed_surface() const override {
        return m_A.is_closed_surface() and m_B.is_closed_surface();
    }

    bool is_surface_point(const point& world_point) const override;
    element_type get_object_extant(void) const override;

protected:
    const object& m_A;
    const object& m_B;
    const type m_type;
};
}  // namespace objects
}  // namespace raytrace