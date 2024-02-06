#include "raytrace/camera.hpp"

#include <basal/exception.hpp>
#include <cassert>
#include <linalg/linalg.hpp>

#include "iso/radians.hpp"

// When true, enable printing some intermediate results in the math
constexpr static bool debug = false;

namespace raytrace {

camera::camera(size_t image_height, size_t image_width, iso::degrees field_of_view)
    : entity{}
    , capture{image_height, image_width}
    , mask{image_height, image_width}
    , m_intrinsics{matrix::identity(raytrace::dimensions, raytrace::dimensions)}
    , m_pixel_scale{1.0}  // will be computed in a second
    , m_field_of_view{field_of_view}
    , m_world_look_at{1.0, 0.0, 0.0}
    , m_world_look{m_world_look_at - m_world_position}  // position starts at 0,0,0
    , m_world_up{R3::basis::Z}
    , m_world_left{R3::basis::Y} {
    // compute f so that pixel_scale will remain 1 at first.
    iso::radians rfov;
    iso::convert(rfov, m_field_of_view);
    precision f = (image_width / 2) / std::tan(rfov.value / 2.0);
    m_world_look_at.x = f;

    // the rotation from camera (+Z forward, -Y up) to world frame (+Z up, +X forward)
    iso::radians phi(iso::pi / 2);
    matrix r1 = geometry::rotation(R3::basis::Z, -phi);
    matrix r2 = geometry::rotation(R3::basis::Y, phi);
    m_camera_to_object_rotation = r2 * r1;

    // rotation matrix always must have a determinant of 1
    assert(basal::nearly_equals(det(m_camera_to_object_rotation), 1.0));

    // we can't move anything until the camera to world rotation has been computed, as we use it.
    move_to(m_world_position, m_world_look_at);
    // initialize it to all white
    mask.for_each ([](size_t, size_t, uint8_t& pixel) -> void { pixel = image::AAA_MASK_DISABLED; });
}

camera::camera(camera&& other)
    : entity{other.position()}                            // copy
    , capture{other.capture.height, other.capture.width}  // create our own
    , mask{other.mask.height, other.mask.width}
    , m_intrinsics{matrix::identity(raytrace::dimensions, raytrace::dimensions)}
    , m_pixel_scale{1.0}  // will be computed in a second
    , m_field_of_view{other.m_field_of_view}
    , m_world_look_at{1.0, 0.0, 0.0}
    , m_world_look{other.m_world_look_at - other.m_world_position}  // position starts at 0,0,0
    , m_world_up{R3::basis::Z}
    , m_world_left{R3::basis::Y} {
}

void camera::move_to(const point& look_from, const point& look_at) {
    using namespace iso::operators;
    // incoming values are *WORLD* coordinates

    // set the look vector (which will be reported as "forward")
    vector world_look = look_at - look_from;
    if constexpr (debug) {
        std::cout << "Look From " << look_from << std::endl;
        std::cout << "Look At " << look_at << std::endl;
        std::cout << "World Look " << world_look << std::endl;
    }
    if (world_look.magnitude() == 0.0) {
        throw basal::exception("From and At are the same point", __FILE__, __LINE__);
    }
    // create the left by crossing with up (+Z)
    vector world_up = R3::basis::Z;
    // check that the vectors aren't the same
    if (world_look.normalized() == world_up) {
        throw basal::exception("Can't look straight up +Z or down -Z", __FILE__, __LINE__);
    }

    vector world_left = R3::cross(world_up, world_look);
    if constexpr (debug) {
        std::cout << "World Left " << world_left << std::endl;
    }
    if (world_left.magnitude() == 0.0) {
        throw basal::exception(
            "Look and Up are co-linear, unable to cross. This camera assumes +Z is up in world coordinates", __FILE__,
            __LINE__);
    }
    //==========================================================
    // now update the variables that we've passed the exceptions
    m_world_position = look_from;
    m_world_look_at = look_at;
    m_world_look = world_look;
    m_world_left = world_left;
    precision image_distance = m_world_look.magnitude();
    // which we'll use to make the actual up
    m_world_up = R3::cross(m_world_look, world_left);
    if constexpr (debug) {
        std::cout << "World Up " << m_world_up << std::endl;
    }
    // convert the work look into a point in R3
    R3::point cartesian_world_point(m_world_look[0], m_world_look[1], m_world_look[2]);
    // (r, theta, phi)
    R3::point spherical_world_point = cartesian_to_spherical(cartesian_world_point);
    iso::radians rx{0};                                       // no barrel roll
    iso::radians ry{spherical_world_point[2] - iso::pi / 2};  // phi with "level" at XY plane
    iso::radians rz{spherical_world_point[1]};                // theta
    if constexpr (debug) {
        std::cout << "Rotation X (Barrel)" << rx.value << std::endl;
        std::cout << "Rotation Y (Tilt)" << ry.value << std::endl;
        std::cout << "Rotation Z (Pan)" << rz.value << std::endl;
    }
    // tilt can only be just shy of up or down
    basal::exception::throw_unless((iso::pi > ry.value and ry.value > -iso::pi / 2), __FILE__, __LINE__,
                                   " Tilt must be +PI/2 > %lf > -PI/2\r\n", ry.value);
    // no rotation should ever be NaN
    assert(not basal::is_nan(rx.value) and not basal::is_nan(ry.value) and not basal::is_nan(rz.value));
    // move the camera's entity base class by these values
    rotation(rx, ry, rz);

    // update the intrinsics which converts the focal distance into scaling for pixel
    iso::radians phi;  // half of the field of view
    iso::convert(phi, m_field_of_view * 0.5);
    precision w = precision(capture.width);
    precision h = precision(capture.height);
    m_pixel_scale = 2.0 * image_distance * std::tan(phi.value) / w;
    m_intrinsics[0][0] = m_pixel_scale;
    m_intrinsics[1][1] = m_pixel_scale;
    m_intrinsics[2][2] = image_distance;
    m_intrinsics[0][2] = -((w / 2.0) * m_pixel_scale);  // primary point x
    m_intrinsics[1][2] = -((h / 2.0) * m_pixel_scale);  // primary point y

    // now verify the look_at by casting a ray through the principal point and determine what t the look_at is at
    // (should be zero).
    image::point P{(precision)capture.width / 2, (precision)capture.height / 2};
    ray world_ray = cast(P);
    // the point we're looking at had better be where this ray starts
    precision t = basal::nan;
    geometry::R3::line world_line = as_line(world_ray);
    // t should be zero, as it is the position of the world ray and line
    bool principal_found_at_look_at = world_line.solve(look_at, t);
    if constexpr (debug) {
        print(">>> Camera State at the end of move_to()\r\n");
        std::cout << "Look at " << look_at << " is at t=" << t << " Found?=" << principal_found_at_look_at << std::endl;
        std::cout << "######### END OF MOVE ######### " << std::endl;
        basal::exception::throw_unless(principal_found_at_look_at, __FILE__, __LINE__);
    }
}

void camera::print(const char str[]) const {
    printf("%s", str);
    m_intrinsics.print("Camera Intrinsics");
    m_world_position.print("Camera Look From");
    m_world_look_at.print("Camera Look at");
    m_world_look.print("Camera Forward");
    m_world_left.print("Camera Left");
    m_world_up.print("Camera Up");
    m_camera_to_object_rotation.print("Camera to Object Rotation");
    m_rotation.print("Object to World Rotation");
}

ray camera::cast(const image::point& image_point) const {
    constexpr static bool cast_debug = false;
    // the input point may have some subsampling offset around
    // the point, so don't expect whole numbers
    // the point is in "raster" or "pixel" coordinates, homogenize it (add a "z")
    if constexpr (cast_debug) {
        std::cout << "+++ CAST() " << std::endl;
        std::cout << "\tImage Point " << image_point << std::endl;
    }
    point hg_image_point(image_point);
    if constexpr (cast_debug) {
        std::cout << "\tImage Point Homogenized " << hg_image_point << std::endl;
    }
    // convert to the x.y,z of the camera coordinates
    // the image plane is in the x,y camera coordinates plane but at z=1
    // basically there has to be a assumed arrangement
    // of the camera point and the image plane.
    geometry::point camera_point(m_intrinsics * hg_image_point);
    if constexpr (cast_debug) {
        std::cout << "\tCamera Intrinsics " << m_intrinsics << std::endl;
        std::cout << "\tNew Camera Point " << camera_point << std::endl;
        std::cout << "\tCamera to Object Rotation " << m_camera_to_object_rotation << std::endl;
    }
    // now rotate and translate the camera point into the world point
    geometry::point object_point = m_camera_to_object_rotation * camera_point;
    // go from object point to world point
    geometry::point world_point = forward_transform(object_point);
    // now that we know where the world_point is for this image point,
    // find the vector from the camera origin (in world coord) to the world_point on the image plane
    vector world_direction = world_point - position();
    if constexpr (cast_debug) {
        std::cout << "\tCamera Object Point " << object_point << std::endl;
        std::cout << "\tCamera Transform to World " << m_transform << std::endl;
        std::cout << "\tCamera World Point " << world_point << std::endl;
        std::cout << "\tWorld Direction " << world_direction << std::endl;
        std::cout << "--- CAST() " << std::endl;
    }
    // now base the ray starting at the image plane with the vector from above
    ray world_ray(world_point, world_direction.normalize());
    // increment the statistics
    statistics::get().cast_rays_from_camera++;
    // that's it
    return world_ray;
}

ray camera::forward() const {
    return ray(position(), m_world_look);
}

ray camera::up() const {
    return ray(position(), m_world_up);
}

ray camera::left() const {
    return ray(position(), m_world_left);
}

const point& camera::at() const {
    return m_world_look_at;
}

void camera::at(const point& look_at) {
    move_to(position(), look_at);
}

void camera::position(const point& look_from) {
    move_to(look_from, m_world_look_at);
}

const point& camera::position() const {
    return m_world_position;
}

const matrix& camera::intrinsics() const {
    return m_intrinsics;
}

}  // namespace raytrace
