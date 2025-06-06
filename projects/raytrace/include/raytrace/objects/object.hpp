#pragma once

#include <iostream>

#include "basal/basal.hpp"
#include "raytrace/entity.hpp"
#include "raytrace/laws.hpp"
#include "raytrace/bounds.hpp"
#include "raytrace/mediums/medium.hpp"
#include "raytrace/mediums/plain.hpp"

namespace raytrace {
/// The namespace to contain all (spatial) objects
namespace objects {

using namespace linalg;
using namespace geometry;
using namespace raytrace::mediums;

/// An enumeration of the various types of objects in the raytracer.
/// @note Used for debugging. The values are not guaranteed to be repeated across builds.
enum class Type : int {
    None = 0,
    Cone,
    Cuboid,
    Cylinder,
    Ellipsoid,
    EllipticalCone,
    EllipticalCylinder,
    Face,
    Hyperboloid,
    Model,
    Overlap,
    Paraboloid,
    Plane,
    Polygon,
    Pyramid,
    Quadratic,
    Ring,
    Sphere,
    Square,
    Torus,
    Wall,
};

/// Indicates which sort of 3d topology type the
enum class ExtentType : int {
    Finite = 0,  ///< The surface is finite in all dimensions (e.g., cuboid, sphere)
    Infinite,    ///< The surface is infinite in one or more dimensions or extents
};

/// A template for wrapping the concept of an object, which can have a surface and material properties.
template <size_t DIMS>
class object_
    : public entity_<DIMS>
    , public basal::printable {
public:
    object_()
        : entity_<DIMS>()
        , m_type{Type::None}  // default to none
        , m_max_collisions{0}
        , m_has_definite_volume{false}
        , m_medium(&mediums::dull)
        , m_surface_scale{1.0_p, 1.0_p} {
    }

    object_(point const& center, size_t collisions, Type type, bool closed = false)
        : entity_<DIMS>(center)
        , m_type{type}
        , m_max_collisions{collisions}
        , m_has_definite_volume{collisions > 1 ? closed : false}
        , m_medium{&mediums::dull}
        , m_surface_scale{1.0_p, 1.0_p} {
    }

    object_(point&& center, size_t collisions, Type type, bool closed = false)
        : entity_<DIMS>(std::move(center))
        , m_type{type}
        , m_max_collisions{collisions}
        , m_has_definite_volume{collisions > 1 ? closed : false}
        , m_medium{&mediums::dull} {
    }

    /// Copy Constructor for the object. This is allowed as the medium is constant!
    object_(object_ const& that)
        : entity_<DIMS>(that)
        , m_type{that.m_type}
        , m_max_collisions{that.m_max_collisions}
        , m_has_definite_volume{that.m_has_definite_volume}
        , m_medium(that.m_medium) {
    }

    /// Move Constructor for the object. This is allowed as the medium is constant!
    object_(object_&& that)
        : entity_<DIMS>(that)
        , m_type{that.m_type}  // can't move the type
        , m_max_collisions{that.m_max_collisions}
        , m_has_definite_volume{that.m_has_definite_volume}
        , m_medium(that.m_medium) {
        that.m_type = Type::None;            // clear the type from the moved object
        that.m_max_collisions = 0;           // clear the collisions from the moved object
        that.m_has_definite_volume = false;  // clear the closed surface from the moved object
        that.m_medium = nullptr;             // clear the medium from the moved object
    }

    /// Copy Assignment for the object. This is allowed as the medium is constant!
    object_& operator=(object_ const& that) {
        entity_<DIMS>::operator=(that);
        m_max_collisions = that.m_max_collisions;
        m_has_definite_volume = that.m_has_definite_volume;
        m_medium = that.m_medium;
        return *this;
    }

    object_& operator=(object_&& that) {
        entity_<DIMS>::operator=(that);
        m_max_collisions = that.m_max_collisions;
        m_has_definite_volume = that.m_has_definite_volume;
        m_medium = that.m_medium;  // copy is sufficient as the medium is constant
        that.m_medium = nullptr;   // clear the medium from the moved object
        return *this;
    }

    virtual ~object_() {
        m_medium = nullptr;  // forget the medium
    }

    /// Gives a read-only version of the medium of the object
    medium const& material() const {
        basal::exception::throw_if(m_medium == nullptr, __FILE__, __LINE__, "Can not request a nullptr.");
        return *m_medium;
    }

    /// Sets the material to the given parameter medium
    virtual void material(medium const* new_medium) {
        basal::exception::throw_if(new_medium == nullptr, __FILE__, __LINE__, "Can not set a nullptr to material()");
        m_medium = new_medium;
    }

    /// Computes the reflection vector at the surface point P given the incident vector I
    /// @param I The incident vector
    /// @param N The normal to the surface
    /// @param P The surface point
    /// @note Maintains the space the inputs were in.
    vector reflection(vector const& I, vector const& N, point const& P) const {
        vector nI = I.normalized();  // MAYBE (Optimization) shouldn't this already be normalized?
        vector R = laws::reflection(N, nI);
        if (m_medium) {
            R = R + m_medium->perturbation(P);
        }
        return R;
    }

    /// Computes the reflection ray at the surface point P given the incident ray I
    /// @param I The incident ray
    /// @param N The normal to the surface
    /// @param P The surface point
    /// @note Maintains the space the inputs were in.
    ray reflection(ray const& I, vector const& N, point const& P) const {
        return ray(P, reflection(I.direction(), N, P));
    }

    /// Computes the refraction vector at the surface point
    /// @param I The incident vector
    /// @param N The normal to the surface
    /// @param P The surface point
    /// @param nu1 Material Refractivity Index
    /// @param nu2 Material Refractivity Index
    ///
    vector refraction(vector const& I, vector const& N, precision nu1, precision nu2) const {
        vector nI = I.normalized();  // MAYBE (Optimization) shouldn't this already be normalized?
        // if the Incident and the Normal have a positive dot then they are not arranged properly
        if (dot(N, I) > 0) {
            return laws::snell(-N, nI, nu1, nu2);
        } else {
            return laws::snell(N, nI, nu1, nu2);
        }
    }

    /// Computes the refraction ray at the surface point P.
    /// @param I The incident ray
    /// @param N The normal to the surface
    /// @param P The surface point
    /// @param nu1 Material Refractivity Index
    /// @param nu2 Material Refractivity Index
    ///
    ray refraction(ray const& I, vector const& N, point const& P, precision nu1, precision nu2) const {
        return ray(P, refraction(I.direction(), N, nu1, nu2));
    }

    /// Returns the normal to the surface given an world space point which is presumed to
    /// be the collision point, thus on the surface.
    /// @param world_surface_point The point on the object to use.
    /// @warning It is assumed that the point given is an intersection point.
    /// @retval R3::null indicates that the point is not on the surface.
    vector normal(point const& world_surface_point) const {
        point object_surface_point = entity::reverse_transform(world_surface_point);
        return entity::forward_transform(normal_(object_surface_point));
    }

    /// Contains all the information about a collision with an object.
    /// This is to facilitate the overlap objects and other more complex situations.
    /// During the collision process the points are all in object space
    /// and are converted to world_space when returned.
    struct hit {
        hit() : intersect{}, distance{std::numeric_limits<precision>::infinity()}, normal{}, object{nullptr} {
        }
        hit(hit const& that) = default;
        hit(intersection i, precision d, vector n, object_ const* o) : intersect{i}, distance{d}, normal{n}, object{o} {
        }
        intersection intersect;  //!< The type of intersection (includes the point)
        precision distance;      //!< The distance along the ray of intersection.
        vector normal;           //!< The normal at the point along the line
        object_ const* object;   //!< The pointer to the object that was hit

        /// A hit can be assigned from another hit
        hit& operator=(hit const& that) {
            intersect = that.intersect;
            distance = that.distance;
            normal = that.normal;
            object = that.object;
            return *this;
        }

        /// A hit can be moved from another hit
        hit& operator=(hit&& that) {
            intersect = std::move(that.intersect);
            distance = std::move(that.distance);
            normal = std::move(that.normal);
            object = std::move(that.object);
            return *this;
        }

        /// A hit is equal to another if at least the point, distance and object are the same
        bool operator==(hit const& that) const {
            using namespace geometry::operators;
            bool approximate_distance = basal::nearly_equals(this->distance, that.distance);
            bool same_intersection = (this->intersect == that.intersect);
            bool same_object = (this->object == that.object);
            // the normal is not compared
            return (approximate_distance and same_intersection and same_object);
        }

        bool operator!=(hit const& that) const {
            return not operator==(that);
        }

        /// Used for sorting hits by distance
        bool operator<(hit const& that) const {
            return (this->distance < that.distance);
        }

        /// Used for sorting hits by distance
        bool operator>(hit const& that) const {
            return (this->distance > that.distance);
        }

        friend std::ostream& operator<<(std::ostream& os, hit const& h) {
            return os << "hit{intersect=" << h.intersect << ", distance=" << h.distance << ", normal=" << h.normal
                      << ", object=" << h.object << "}";
        }
    };
    /// A set of distances along the world_ray which collide with the object, could be many.
    using hits = std::vector<hit>;

    /// Returns all the intersections with this object along the ray (extended as a line).
    /// @param object_ray The ray in object space.
    /// @return The unordered set of intersection distances along the world ray.
    virtual hits collisions_along(ray const& object_ray) const = 0;

    /// Returns an intersection with this object
    /// @param world_ray The ray from the world space to test the intersection with.
    /// @return Returns the first intersection with the object.
    ///
    virtual hit intersect(ray const& world_ray) const {
        hit closest;
        if constexpr (enforce_contracts) {
            /// @note While we could be pedantic about having a unit normal, it doesn't really stop us from working.
            basal::exception::throw_unless(basal::nearly_equals(world_ray.direction().quadrance(), 1.0_p), __FILE__,
                                           __LINE__, "The ray must have a unit vector");
        }
        ray object_ray = entity_<DIMS>::reverse_transform(world_ray);
        // get the set of all collisions with the object
        hits collisions = collisions_along(object_ray);
        // ts could contain NaN and +Inf/-Inf
        for (auto& collision : collisions) {
            if (basal::is_nan(collision.distance)) {
                continue;
            }
            basal::exception::throw_if(collision.normal == R3::null, __FILE__, __LINE__, "The normal can't be null");
            if constexpr (can_ray_origin_be_collision) {
                if (basal::nearly_zero(collision.distance)) {
                    vector const N = normal_(object_ray.location());
                    vector const& I = object_ray.direction();
                    precision d = dot(N, I);
                    if (d < 0.0_p) {  // the ray points "into" the material so it's a collision
                        // however if the material is transparent, that's ok and it should not count as a collision
                        if (m_medium and m_medium->refractive_index(object_ray.location()) > 0.0_p) {
                            continue;
                        }
                        closest = collision;
                        closest.intersect = intersection{entity_<DIMS>::forward_transform(as_point(closest.intersect))};
                        closest.normal = entity_<DIMS>::forward_transform(closest.normal);
                        break;
                    } else {
                        // the ray points "out" of the material so no collision
                    }
                }
            }
            if (basal::epsilon < collision.distance and collision.distance < closest.distance) {
                closest = collision;
                // update the collision point to be in world space
                closest.intersect = intersection{entity_<DIMS>::forward_transform(as_point(closest.intersect))};
                closest.normal = entity_<DIMS>::forward_transform(closest.normal);
            }
        }
        return closest;
    }

    /// Maps a surface point (in R3 object space) to a image::point in u,v coordinates (R2)
    /// @param object_surface_point The point on the surface of the object in object space coordinates
    virtual image::point map(point const& object_surface_point) const = 0;

    /// Returns the maximum number of surface collisions that the object can return.
    /// It is assumed that the actual collisions could be between 0 and this number.
    ///
    virtual inline size_t max_collisions() const {
        return m_max_collisions;
    }

    /// @return true if the object is a definite volume (e.g., sphere, cuboid, etc.) and false if it is an open surface
    /// like a plane
    virtual inline bool has_definite_volume() const {
        return m_has_definite_volume;
    }

    /// @brief Used to determine if an open surface and ray could ever possibly intersect.
    /// @param world_ray The ray in world space to test against.
    /// @return False if the ray is not along the infinite extent. Closed surfaces will always return false.
    virtual inline bool is_along_infinite_extent(ray const& world_ray __attribute__((unused))) const {
        return false;
    }

    /// @brief Determines if a point is considered "outside" the surface
    /// by looking at the normal at that point.
    /// @param world_point The point in world space.
    /// @warning the default implementation only works for objects with their centers
    /// on their insides and not concave in some way. Toroids and other objects with collisions > 2 will have
    /// have their own implementations.
    /// @return True for point outside the object, false for points on the surface or inside.
    virtual bool is_outside(point const& world_point) const {
        raytrace::vector const& N = normal(world_point);
        raytrace::point const& P = entity_<DIMS>::position();
        raytrace::vector V = world_point - P;
        precision d = dot(N, V);
        return (d > 0.0);  // if the dot of the Normal on the point from center
    }

    /// Given a world point verify that the point is on the surface of the object.
    virtual bool is_surface_point(raytrace::point const& world_point __attribute__((unused))) const {
        return false;  // override to correct
    };

    /// Computes the Axis Aligned Bounding Box in World Coordinates for this object.
    Bounds get_world_bounds(void) const {
        // finds the object's maximum radial distance from the object origin
        auto r = get_object_extent();
        basal::exception::throw_if(std::isnan(r) or r < 0.0_p, __FILE__, __LINE__, "Got a %lf from an extent\r\n", r);
        if (std::isinf(r)) {
            return Bounds{};  // infinite bounds
        }
        // the bounding box should not take into account the rotation of the object, this will cause max < min.
        auto p0 = raytrace::point{-r, -r, -r} + (entity_<DIMS>::position() - R3::origin);
        auto p1 = raytrace::point{r, r, r} + (entity_<DIMS>::position() - R3::origin);
        if constexpr (debug::bounds) {
            std::cout << "Object position " << entity_<3>::position() << " extent: " << r << " p0 " << p0 << " p1 "
                      << p1 << std::endl;
        }
        // constructs a local with min (inclusive) and max (exclusive)
        return Bounds{p0, p1};
    }

    /// Returns the maximum radial distance from the object origin on the surface of the object.
    /// @warning Objects which are not closed or have infinite dimensionality will return std::nan
    virtual precision get_object_extent(void) const = 0;

    /// Allows setting the UV scaling factors independently.
    /// @note Used when the object is infinite in some dimension and that dimension can't be used to scale the uv
    virtual void set_surface_scale(precision u, precision v) {
        m_surface_scale.u = u;
        m_surface_scale.v = v;
    }

    /// Each object subclass must overload this and return the unique type they are. This is purely used for debugging.
    Type get_type() const {
        return m_type;
    }

    friend std::ostream& operator<<(std::ostream& os, object_ const& obj) {
        obj.print(os, "object");
        os << "object_{type=" << static_cast<int>(obj.m_type) << ", max_collisions=" << obj.m_max_collisions
           << ", definite volume=" << obj.m_has_definite_volume << ", medium=" << (obj.m_medium ? "true" : "false")
           << ", surface_scale={" << obj.m_surface_scale.u << ", " << obj.m_surface_scale.v << "}}";
        return os;
    }

protected:
    /// @brief Computes the normal to the surface given an object space point which is presumed to be the collision
    /// point, thus on the surface.
    /// @param object_surface_point
    /// @return
    virtual vector normal_(point const& object_surface_point) const = 0;

    /// the type of object, used for debugging and determining the type of in debugging windows when viewing the object
    Type m_type;
    /// The maximum number of collisions with the surface of this object
    size_t m_max_collisions;
    /// Some objects may return more than 1 collisions but do not enclose a definite volume like a sphere. A plane for
    /// example does not enclose a definite volume.
    bool m_has_definite_volume;
    /// The pointer to the medium to use
    raytrace::mediums::medium const* m_medium;
    /// The std::bind or used to reference the instance of the mapping function
    std::function<geometry::R2::point(geometry::R3::point const&)> m_bound_map;
    /// The UV surface scaling factors
    struct {
        precision u;  ///< The scaling factor for U
        precision v;  ///< The scaling factor for V
    } m_surface_scale;
};

/// In Raytracing we only consider 3D objects
using object = object_<dimensions>;

/// A single hit from the object type
using hit = object::hit;

/// A set of hits (from the object type)
using hits = object::hits;

}  // namespace objects
}  // namespace raytrace
