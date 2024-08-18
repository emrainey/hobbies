#include "raytrace/objects/object.hpp"

namespace raytrace {
namespace objects {
/// @brief A collection of objects which are not necessarily connected, but are grouped together
/// for the purpose of translating, rotating and scaling.
/// Nearly all the methods of object have to be extended in order to work with a group.
class group : public object {
public:
    group() = default;
    virtual ~group() = default;

    /// Adds an object to the group
    /// @param obj The object to add to the group
    void add_object(object* obj) {
        m_objects.append(obj);
        // object's position is now relative to the group position().
    }

    void rotate(iso::radians const& angle, vector const& axis) override {
        // all objects in the group will be rotated around the group's position
        // FIXME rotate around a position of the group, not object center.
    }

    /// Translates all objects in the set together by a vector. This is a convenience function.
    /// @param v The vector to apply to all objects in the group
    void move_by(vector const& v) {
        for (auto& obj : m_objects) {
            obj->move_by(v);
        }
    }

    /// Asks each sub-object if the point is on their surface. If any object returns true, then the group
    bool is_surface_point(raytrace::point const& world_point) const override {
        for (auto& obj : m_objects) {
            if (obj->is_surface_point(world_point)) {
                return true;
            }
        }
        return false;
    }

    /// This must find the extent of the whole set of objects in the group.
    /// It does this by taking each objects extent and their center and treating them as spheres.
    /// Then finding the widest or longest extent across those sets of spheres. It could also find a centroid
    /// of the group, and compute the farthest distance from the centroid and that would count as the extent.
    precision get_object_extent(void) const override {
        // the position() of the group should be the centroid of the group
    }

protected:
    /// The list of objects in the group
    std::vector<object*> m_objects;
};
}  // namespace objects
}  // namespace raytrace