#pragma once

#include "geometry/extra_math.hpp"
#include "linalg/matrix.hpp"
#include "raytrace/types.hpp"

namespace raytrace {

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

/// Contains the internal attributes that 3D entities will need. Position, orientation, scaling
template <size_t DIMS>
class entity_ {
public:
    entity_()
        : m_world_position{}
        , m_rotation{matrix::identity(DIMS, DIMS)}
        , m_inv_rotation{matrix::identity(DIMS, DIMS)}
        , m_scaling{{1.0_p, 1.0_p, 1.0_p}}
        , m_transform{matrix::identity(DIMS + 1, DIMS + 1)}
        , m_inv_transform{matrix::identity(DIMS + 1, DIMS + 1)} {
    }

    entity_(point const& position) : entity_{} {
        m_world_position = position;
        compute_transforms();
    }

    entity_(point&& position) : entity_{} {
        m_world_position = std::move(position);
        compute_transforms();
    }

    entity_(entity_ const&) = delete;
    entity_(entity_&&) = delete;
    entity_& operator=(entity_ const&) = delete;
    entity_& operator=(entity_&&) = delete;
    virtual ~entity_() = default;

    /// Returns the current position as a point in world space!
    /// @warning Should not be used in object space calculations!
    virtual point const& position() const {
        return m_world_position;
    }

    /// Returns the current position as a translation vector from the origin
    vector const translation() const {
        return vector{{m_world_position[0], m_world_position[1], m_world_position[2]}};
    }

    /// Returns the current rotation
    matrix const& rotation() const {
        return m_rotation;
    }

    /// Sets the position
    virtual void position(point const& world_point) {
        m_world_position = world_point;
        compute_transforms();
    }

    /// Sets the rotation as a single matrix
    void rotation(matrix const& nr) {
        m_rotation = nr;
        m_inv_rotation = nr.inverse();
        compute_transforms();
    }

    /// Moves an object by a world space vector
    void move_by(vector const& world_vector) {
        m_world_position += world_vector;
        compute_transforms();
    }

    /// Sets the rotation matrix by specifying the rotation around the x, then y then z axis.
    /// This is the roll, pitch, yaw order.
    /// @warning This may result in a non-unique rotation!
    void rotation(iso::radians xr, iso::radians yr, iso::radians zr) {
        matrix Rx = geometry::rotation(R3::basis::X, xr);
        matrix Ry = geometry::rotation(R3::basis::Y, yr);
        matrix Rz = geometry::rotation(R3::basis::Z, zr);
        m_rotation = (Rz * Ry) * Rx;
        // this is used to rotate vectors back into object space.
        m_inv_rotation = m_rotation.inverse();
        compute_transforms();
    }

    /// Sets the rotation matrix by specifying the rotation around the x, then y then z axis.
    /// This is the roll, pitch, yaw order.
    /// @warning This may result in a non-unique rotation!
    void rotation(iso::degrees x, iso::degrees y, iso::degrees z) {
        iso::radians x_r, y_r, z_r;
        iso::convert(x_r, x);
        iso::convert(y_r, y);
        iso::convert(z_r, z);
        rotation(x_r, y_r, z_r);
    }

    /// Finds the point in world space given the object point
    raytrace::point forward_transform(raytrace::point const& object_point) const {
        geometry::point_<4> o(object_point);
        geometry::point_<4> w(m_transform * o);
        return point(w[0], w[1], w[2]);
    }

    /// Rotates the vector into the world space from the object space
    raytrace::vector forward_transform(raytrace::vector const& vec) const {
        return vector(m_rotation * vec);
    }

    /// Transforms a object ray into a world ray
    raytrace::ray forward_transform(raytrace::ray const& object_ray) const {
        raytrace::point world_point = forward_transform(object_ray.location());
        vector world_vector = forward_transform(object_ray.direction());
        return ray(world_point, world_vector);
    }

    /// Finds the point in object space which maps to the given world space point
    raytrace::point reverse_transform(raytrace::point const& world_point) const {
        geometry::point_<4> w(world_point);          // create homogenized point
        geometry::point_<4> o(m_inv_transform * w);  // 4x4 mult
        return point(o[0], o[1], o[2]);              // drop the last var
    }

    /// Rotates the vector into the object space.
    raytrace::vector reverse_transform(raytrace::vector const& vec) const {
        vector v = m_inv_rotation * vec;
        return vector(v);
    }

    /// Transforms the world space ray into the object space ray
    raytrace::ray reverse_transform(raytrace::ray const& world_ray) const {
        point object_point = reverse_transform(world_ray.location());
        vector object_vector = reverse_transform(world_ray.direction());
        return ray(object_point, object_vector);
    }

    /// Updates the scaling values
    void scale(precision x, precision y, precision z) {
        m_scaling[0] = x;
        m_scaling[1] = y;
        m_scaling[2] = z;
        compute_transforms();
    }

protected:
    /// Creates the transform matrix and it's inverse
    void compute_transforms() {
        matrix t = matrix::identity(DIMS + 1, DIMS + 1);
        t[0][3] = m_world_position.x;
        t[1][3] = m_world_position.y;
        t[2][3] = m_world_position.z;
        matrix s = matrix::identity(DIMS + 1, DIMS + 1);
        s[0][0] = m_scaling[0];
        s[1][1] = m_scaling[1];
        s[2][2] = m_scaling[2];
        // expand rotation to 4x4
        matrix r = matrix::identity(DIMS + 1, DIMS + 1);
        // copy the rotation into r at 0,0 to 2,2
        m_rotation.assignInto(r, 0, 0);

        // the order applied is backwards from the multiplication order
        // Rotate, Scale, then Translate.
        m_transform = t * (r * s);
        m_inv_transform = m_transform.inverse();
    }

    /// The position of the object in 3d space
    raytrace::point m_world_position;

    /// The rotation around it's own position, not around the origin
    matrix m_rotation;

    /// The inverse rotation matrix.
    matrix m_inv_rotation;

    /// The Scaling vector
    raytrace::vector m_scaling;

    /// Contains the translation, scaling, and rotation as a single matrix
    matrix m_transform;

    /// Contains the inverse transform
    matrix m_inv_transform;
};

///  Raytracing uses 3D entities
using entity = entity_<dimensions>;

}  // namespace raytrace