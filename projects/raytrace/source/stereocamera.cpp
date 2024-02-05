#include "basal/exception.hpp"
#include "raytrace/stereocamera.hpp"

namespace raytrace {

stereo_camera::stereo_camera(size_t image_height, size_t image_width, iso::degrees field_of_view,
                             double camera_separation, Layout layout)
    : m_separation{camera_separation}
    , m_toe_in{0u}
    , m_layout{layout}
    , m_cameras{{image_height, image_width, field_of_view},
                {image_height, image_width, field_of_view}} {
    basal::exception::throw_if((m_separation < 0.0), __FILE__, __LINE__, "The camera separation %lf must be greater than 0.", m_separation);
}

void stereo_camera::move_to(const point& look_from, const point& look_at) {
    raytrace::vector look = (look_at - look_from).normalized();
    // presume that +Z is the up vector for now and normalize the value
    raytrace::vector right = cross(look, geometry::R3::basis::Z).normalized();
    // compute the actual up vector
    raytrace::vector up = cross(right, look);
    // the offsets from the look_from
    raytrace::vector left_camera_offset = -right * (m_separation * 0.5);
    raytrace::vector right_camera_offset = right * (m_separation * 0.5);
    // using the offsets, compute the positions
    raytrace::point left_camera_position = look_from + left_camera_offset;
    raytrace::point right_camera_position = look_from + right_camera_offset;
    // find the normalized look vector for each eye
    raytrace::vector left_looking = (look_at - left_camera_position).normalized();
    raytrace::vector right_looking = (look_at - right_camera_position).normalized();
    // find the principal point for each eye
    raytrace::point left_principal_point = left_camera_position + left_looking;
    raytrace::point right_principal_point = right_camera_position + right_looking;
    // move both cameras to the correct positions
    m_cameras[0].move_to(left_camera_position, left_principal_point);
    m_cameras[1].move_to(right_camera_position, right_principal_point);
    // compute the toe-in left the left camera, it should be the same since we used the same right vector
    auto left_look = look_at - left_camera_position;
    m_toe_in = angle(look, left_look);
    position(look_from);
    m_look_at = look_at;
}

void stereo_camera::print(const char str[]) const {
    m_cameras[0].print(str);
    m_cameras[1].print(str);
}

image stereo_camera::merge_images(void) const noexcept {
    if (m_layout == Layout::LeftRight) {
        // the width is the sum of the two widths (side by side)
        raytrace::image merged{m_cameras[0].capture.height, m_cameras[0].capture.width + m_cameras[1].capture.width};
        merged.for_each([&](size_t row, size_t col, fourcc::rgb8& pixel) -> void {
            if (col < m_cameras[0].capture.width) {
                pixel = m_cameras[0].capture.at(row, col);
            } else {
                pixel = m_cameras[1].capture.at(row, col - m_cameras[0].capture.width);
            }
        });
        return merged;
    } else { // if (m_layout == Layout::TopBottom) {
        // the height is the sum of the two heights (top and bottom)
        raytrace::image merged{m_cameras[0].capture.height + m_cameras[1].capture.height, m_cameras[0].capture.width};
        merged.for_each([&](size_t row, size_t col, fourcc::rgb8& pixel) -> void {
            if (row < m_cameras[0].capture.height) {
                pixel = m_cameras[0].capture.at(row, col);
            } else {
                pixel = m_cameras[1].capture.at(row - m_cameras[0].capture.height, col);
            }
        });
        return merged;
    }
}

}  // namespace raytrace
