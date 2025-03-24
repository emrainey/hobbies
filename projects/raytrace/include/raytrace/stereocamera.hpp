#pragma once

#include <basal/printable.hpp>

#include "raytrace/camera.hpp"

namespace raytrace {

/// @brief Stereo Cameras have 2 cameras.
class stereo_camera
    : public entity
    , public basal::printable {
public:
    constexpr static size_t NumberOfCameras = 2;
    using iterator = camera*;
    using iterator_const = camera const*;
    enum class Layout {
        TopBottom,
        LeftRight
    };

    stereo_camera(size_t image_height, size_t image_width, iso::degrees field_of_view, double camera_separation,
                  Layout layout = Layout::LeftRight);
    ~stereo_camera() = default;

    void move_to(point const& look_from, point const& look_at);
    void print(std::ostream&, char const[]) const override;

    /// @brief Returns a pointer to the first camera
    iterator begin(void) noexcept {
        return &m_cameras[0];
    }
    /// @brief Returns a pointer to a const first camera
    iterator_const begin(void) const noexcept {
        return &m_cameras[0];
    }
    /// @brief One past the end of the cameras
    iterator end(void) noexcept {
        return &m_cameras[NumberOfCameras];
    }
    /// @brief One past the end of the cameras
    iterator_const end(void) const noexcept {
        return &m_cameras[NumberOfCameras];
    }

    /// @brief Returns the amount of toe-in for the cameras
    iso::radians toe_in(void) const noexcept {
        return m_toe_in;
    }

    /// Returns the merged image of the two cameras based on the Layout
    image merge_images(void) const noexcept;

    friend std::ostream& operator<<(std::ostream& os, stereo_camera const& cam);

protected:
    precision m_separation;             //!< The unit-less separation between the two cameras.
    point m_look_at;                    //!< The point which both cameras are pointing
    iso::radians m_toe_in;              //!< The inward tilting angle of the views.
    Layout m_layout;                    //!< The layout of the cameras
    camera m_cameras[NumberOfCameras];  //!< The set of cameras
};

}  // namespace raytrace
