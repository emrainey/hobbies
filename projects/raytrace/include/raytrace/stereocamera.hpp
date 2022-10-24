#pragma once

#include <basal/printable.hpp>

#include "raytrace/camera.hpp"

namespace raytrace {

/// @brief Stereo Cameras have 2 cameras.
class stereo_camera
    : public entity
    , public basal::printable {
public:
    stereo_camera(size_t image_height, size_t image_width, iso::degrees field_of_view, double camera_separation);
    ~stereo_camera() = default;

    /** @copydoc camera::move_to */
    void move_to(const point& look_from, const point& look_at);

    /** @copydoc basal::printable::print */
    void print(const char[]) const override final;

protected:
    double m_separation;    //!< The unit-less separation between the two cameras.
    point m_look_at;        //!< The point which both cameras are pointing
    iso::radians m_toe_in;  //!< The inward tilting angle of the views.
    camera m_left;          //!< The left camera
    camera m_right;         //!< The right camera
};

}  // namespace raytrace
