#include "raytrace/stereocamera.hpp"

namespace raytrace {

stereo_camera::stereo_camera(size_t image_height, size_t image_width, iso::degrees field_of_view,
                             double camera_separation)
    : m_separation{camera_separation}
    , m_toe_in{0u}
    , m_left{image_height, image_width, field_of_view}
    , m_right{image_height, image_width, field_of_view} {
}

void stereo_camera::move_to(const point& look_from, const point& look_at) {
    auto look = look_at - look_from;
    // presume that +Z is the up vector for now and normalize the value
    auto right = cross(look, geometry::R3::basis::Z).normalized();
    // compute the actual up vector
    auto up = cross(right, look);
    // the offsets from the look_from
    auto left_camera_offset = -right * m_separation;
    auto right_camera_offset = right * m_separation;
    // using the offsets, compute the positions
    auto left_camera_position = look_from + left_camera_offset;
    auto right_camera_position = look_from + right_camera_offset;
    // move both cameras to the correct positions
    m_left.move_to(left_camera_position, look_at);
    m_right.move_to(right_camera_position, look_at);
    // compute the toe-in left the left camera, it should be the same since we used the same right vector
    auto left_look = look_at - left_camera_position;
    m_toe_in = angle(look, left_look);
    position(look_from);
    m_look_at = look_at;
}

void stereo_camera::print(const char str[]) const {
    m_left.print(str);
    m_right.print(str);
}

}  // namespace raytrace
