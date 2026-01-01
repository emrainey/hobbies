#include "raytrace/objects/group.hpp"

namespace raytrace {
namespace objects {

group::group(point P) : entity{P} {
}

void group::add_object(object* obj) {
    if (obj) {
        m_objects.push_back(obj);
        // Each object keeps it's own _world position_, so we don't need
        // to change that.
    }
}

void group::rotation(iso::radians x, iso::radians y, iso::radians z) {
    // Create rotation matrix
    auto rot = geometry::rotation(z, y, x);

    // Update the group's own rotation
    entity::rotation(x, y, z);

    // Rotate each object around the group's position
    for (auto* obj : m_objects) {
        if (obj != nullptr) {
            // Get object's current position
            point obj_pos = obj->position();

            // Translate to group's local space
            vector offset = obj_pos - entity::position();

            // Apply rotation to the offset
            vector rotated_offset = rot * offset;

            // Translate back to world space
            point new_pos = entity::position() + rotated_offset;

            // Update object position
            obj->position(new_pos);

            // Also rotate the object's own orientation
            obj->rotation(x, y, z);
        }
    }
}

/// Translates all objects in the set together by a vector. This is a convenience function.
/// @param v The vector to apply to all objects in the group
void group::move_by(vector const& world_space_offset) {
    entity::move_by(world_space_offset);
    for (auto& obj : m_objects) {
        // since the vector is in world space it won't need adjusting for all the parts
        obj->move_by(world_space_offset);
    }
}

void group::scale(precision x, precision y, precision z) {
    entity::scale(x, y, z);

    // get the group's origin in world space
    auto group_origin_in_world_space = entity::position();
    for (auto& obj : m_objects) {
        // get the objects current world position
        auto old_object_world_position = obj->position();
        // get the current offset from the group's origin
        auto object_offset = old_object_world_position - group_origin_in_world_space;
        // scale that offset
        auto scaled_offset = vector{object_offset[0] * x, object_offset[1] * y, object_offset[2] * z};
        // compute the new world offset by subtracting the offset from the
        // old position and adding the new scaled offset
        auto new_object_world_position = old_object_world_position - object_offset + scaled_offset;
        // set the object's new world position
        obj->position(new_object_world_position);
        // now perform the scaling on the object itself
        obj->scale(x, y, z);
    }
}

point const& group::position() const {
    return entity::position();
}

void group::position(point const& world_point) {
    // find the offset from the old position to the new position
    auto old_position = entity::position();
    auto offset = world_point - old_position;
    // set the new position for the group
    entity::position(world_point);
    // move all objects in the group by that offset
    for (auto& obj : m_objects) {
        obj->move_by(offset);
    }
}

object_list const& group::get_objects() const {
    return m_objects;
}

}  // namespace objects
}  // namespace raytrace
