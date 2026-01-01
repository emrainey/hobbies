#pragma once

#include "raytrace/objects/object.hpp"

namespace raytrace {
namespace objects {
/// @brief A collection of objects which are not necessarily connected, but are grouped together
/// for the purpose of translating, rotating and scaling.
/// Nearly all the methods of object have to be extended in order to work with a group.
class group : public entity {
public:
    group(point P);
    virtual ~group() = default;

    /// Adds an object to the group
    /// @param obj The object to add to the group
    void add_object(object* obj);

    /// Rotates the group of objects around it's point using right hand rule
    /// @param angle The angle to rotate by
    /// @param axis The axis to rotate around
    void rotation(iso::radians xr, iso::radians yr, iso::radians zr) override;

    /// Scales all objects in the group by the given factors in each dimension
    /// @param x The scaling factor in x
    /// @param y The scaling factor in y
    /// @param z The scaling factor in z
    void scale(precision x, precision y, precision z) override;

    /// Sets the position of the group, moving all objects in the group by the same amount
    /// @param P The new position of the group
    void position(point const& P) override;

    /// Returns the position of the group
    point const& position() const override;

    /// Translates all objects in the set together by a vector. This is a convenience function.
    /// @param v The vector to apply to all objects in the group
    void move_by(vector const& v);

    /// The list of the objects in the group
    object_list const& get_objects() const;

protected:
    /// The list of objects in the group
    object_list m_objects;
};
}  // namespace objects
}  // namespace raytrace