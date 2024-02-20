#pragma once

#include <basal/printable.hpp>
#include <iso/degrees.hpp>
#include <linalg/linalg.hpp>

#include "raytrace/image.hpp"
#include "raytrace/objects/object.hpp"
#include "raytrace/types.hpp"

using namespace basal;

namespace raytrace {
/// The Camera of the Raytrace library. The contains the 'view' which holds the image plane properties.
/// @see http://www.cs.cmu.edu/~16385/s17/Slides/11.1_Camera_matrix.pdf
class camera
    : public entity
    , public basal::printable {
public:
    /// Default constructor
    camera() = delete;

    /// Constructor from Physical Camera concepts
    /// @param image_height The number of pixels of image height.
    /// @param image_width The number of pixels of image width.
    /// @param field_of_view The horizontal field of view of the camera in degrees. Must be less than 180.
    camera(size_t image_height, size_t image_width, iso::degrees field_of_view);

    /// No Copy
    camera(camera const& other) = delete;
    /// No Move
    camera(camera&& other);  // need this for multiview

    /// Default Destructor
    virtual ~camera() = default;

    // Overload Entity Position Set to allow updating internal values
    /// @copydoc entity_::position(point const&)
    void position(point const& p) override final;

    // shouldn't have to overload this but since we overload the setter,
    // it's going to get confused
    /// @copydoc entity_::position
    point const& position() const override final;

    /// @copydoc basal::printable::print
    void print(char const[]) const override final;

    /// Sets the at() location (will update other properties as needed)
    void at(point const& a);

    /// Returns the at() location
    point const& at() const;

    /// Returns the World ray from the 2d point in the image plane.
    /// @param p The point in the image plane in raster coordinates.
    /// @return ray
    ray cast(image::point const& p) const;

    /// Returns the forward ray of the camera (principal point to camera origin)
    /// in world coordinates (used to check the orientation of the camera)
    /// @return ray
    ray forward() const;

    ///
    /// Returns the up ray from the camera point to the upward orientation of
    /// camera in world space. Mainly used to debug the position of the camera after transforms.
    /// Assumes +Z is originally up.
    /// @return ray
    ///
    ray up() const;

    ///      * Returns the left ray from the camera point to the leftward orientation of
/// the camera in the world space. Mainly used to debug the position and orientation of the camera.
/// @return ray
    ray left() const;

    ///
    /// Sets both the position of the camera and the point that the camera is looking at.
    /// Updates other values accordingly. The distance between the look_from and look_at will establish the "scale" of
    /// the scene.
    /// @param [in] look_from The "eye" point of the camera.
    /// @param [in] look_at The image plane principal point (center).
    /// @note This will modify the @ref forward, @ref up and @ref left vectors.
    /// @throw If the distance from to at is zero will throw an exception.
    /// @throw If the cross between the look vector and uppish is zero, will throw an exception
    /// @note Objects behind the principal point will not be rendered as rays will start _from the image plane_.
    ///
    void move_to(point const& look_from, point const& look_at) noexcept(false);

    /// Returns the camera intrinsics for inspection
    linalg::matrix const& intrinsics() const;

    raytrace::image capture;                                ///< The image projected on the plane
    fourcc::image<uint8_t, fourcc::pixel_format::Y8> mask;  ///< The mask of the capture image
protected:
    linalg::matrix m_intrinsics;   ///< Camera Intrinsics
    precision m_pixel_scale;    ///< The scaling factor for sizing pixels in the image plane
    iso::degrees m_field_of_view;  ///< The horizontal field of view of the camera.

    point m_world_look_at;  ///< The location in the world where the principal point will be located
    vector m_world_look;    ///< The computed vector which is the direction of the view in world coordinates
    vector m_world_up;      ///< The computed upwards direction
    vector m_world_left;    ///< The computed left direction

    linalg::matrix
        m_camera_to_object_rotation;  ///< Rotates the Camera Coordinate frame into the Object Coordinate frame.
};
}  // namespace raytrace
