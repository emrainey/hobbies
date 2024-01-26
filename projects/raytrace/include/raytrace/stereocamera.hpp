#pragma once

#include <basal/printable.hpp>

#include "raytrace/camera.hpp"

namespace raytrace {

/// @brief Stereo Cameras have 2 cameras.
class stereo_camera
    : public entity
    , public basal::printable {
public:
    enum class Layout { TopBottom, LeftRight };

    stereo_camera(size_t image_height, size_t image_width, iso::degrees field_of_view, double camera_separation, Layout layout = Layout::LeftRight);
    ~stereo_camera() = default;

    /** @copydoc camera::move_to */
    void move_to(const point& look_from, const point& look_at);

    /** @copydoc basal::printable::print */
    void print(const char[]) const override final;

    camera& first(void) noexcept { return m_first; }
    camera const& first(void) const noexcept { return m_first; }
    camera& second(void) noexcept { return m_second;}
    camera const& second(void) const noexcept { return m_second;}

    /// Returns the merged image of the two cameras based on the Layout
    image merge_images(void) const noexcept;

protected:
    precision m_separation;    //!< The unit-less separation between the two cameras.
    point m_look_at;        //!< The point which both cameras are pointing
    iso::radians m_toe_in;  //!< The inward tilting angle of the views.
    Layout m_layout;        //!< The layout of the cameras
    camera m_first;         //!< The left/top camera
    camera m_second;        //!< The right/bottom camera
};

}  // namespace raytrace
