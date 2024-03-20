#pragma once

#include "basal/basal.hpp"
#include "raytrace/entity.hpp"
#include "raytrace/laws.hpp"
#include "raytrace/mediums/medium.hpp"
#include "raytrace/mediums/plain.hpp"

namespace raytrace {
/// The namespace to contain all (spatial) objects
namespace objects {

/// A flag to control if origin collisions are counted
constexpr bool can_ray_origin_be_collision = true;

using namespace linalg;
using namespace geometry;
using namespace raytrace::mediums;

/// A template for wrapping the concept of an object, which can have a surface and material properties.
template <size_t DIMS>
class object_
    : public entity_<DIMS>
    , public basal::printable {
public:
    object_() : entity_<DIMS>()
              , m_max_collisions{0}
              , m_closed_surface{false}
              , m_medium(&mediums::dull)
              , m_surface_scale{1.0_p, 1.0_p} {
    }

    object_(point const& center, size_t collisions, bool closed = false)
        : entity_<DIMS>(center)
        , m_max_collisions{collisions}
        , m_closed_surface{collisions > 1 ? closed : false}
        , m_medium{&mediums::dull}
        , m_surface_scale{1.0_p, 1.0_p} {
    }

    object_(point&& center, size_t collisions, bool closed = false)
        : entity_<DIMS>(std::move(center))
        , m_max_collisions{collisions}
        , m_closed_surface{collisions > 1 ? closed : false}
        , m_medium{&mediums::dull} {
    }

    /// Copy Constructor for the object. This is allowed as the medium is constant!
    object_(object_ const& that) : entity_<DIMS>(), m_max_collisions{that.m_max_collisions}, m_closed_surface{that.m_closed_surface}, m_medium(that.m_medium) {
    }

    /// Move Constructor for the object. This is allowed as the medium is constant!
    object_(object_&& that) : entity_<DIMS>(), m_max_collisions{that.m_max_collisions}, m_closed_surface{that.m_closed_surface}, m_medium(that.m_medium) {
    }

    /// Copy Assignment for the object. This is allowed as the medium is constant!
    object_& operator=(object_ const& that) {
        m_max_collisions = that.m_max_collisions;
        m_closed_surface = that.m_closed_surface;
        m_medium = that.m_medium;
        return *this;
    }

    object_& operator=(object_&& that) {
        m_max_collisions = that.m_max_collisions;
        m_closed_surface = that.m_closed_surface;
        m_medium = that.m_medium; // copy is sufficient as the medium is constant
        that.m_medium = nullptr; // clear the medium from the moved object
        return *this;
    }

    virtual ~object_() {
        m_medium = nullptr; // forget the medium
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
    /// @note Maintains the space the inputs were in.
    vector reflection(vector const& I, point const& P) const {
        vector N = normal(P);  // MAYBE (Optim) should we already have calculated the normal at this point already?
        vector nI = I.normalized();  // MAYBE (Optim) shouldn't this already be normalized?
        vector R = laws::reflection(N, nI);
        if (m_medium) {
            R = R + m_medium->perturbation(P);
        }
        return R;
    }

    /// Computes the reflection ray at the surface point P given the incident ray I
    /// @note Maintains the space the inputs were in.
    ray reflection(ray const& I, point const& P) const {
        return ray(P, reflection(I.direction(), P));
    }

    /// Computes the refraction vector at the surface point
    /// @param I The incident vector
    /// @param P The surface point
    /// @param nu1 Material Refractivity Index
    /// @param nu2 Material Refractivity Index
    ///
    vector refraction(vector const& I, point const& P, precision nu1, precision nu2) const {
        vector N = normal(P);  // MAYBE (Optim) should we already have calculated the normal at this point already?
        vector nI = I.normalized();  // MAYBE (Optim) shouldn't this already be normalized?
        // if the Incident and the Normal have a positive dot then they are not arranged properly
        if (dot(N, I) > 0) {
            return laws::snell(-N, nI, nu1, nu2);
        } else {
            return laws::snell(N, nI, nu1, nu2);
        }
    }

    /// Computes the refraction ray at the surface point P.
    /// @param I The incident ray
    /// @param P The surface point
    /// @param nu1 Material Refractivity Index
    /// @param nu2 Material Refractivity Index
    ///
    ray refraction(ray const& I, point const& P, precision nu1, precision nu2) const {
        return ray(P, refraction(I.direction(), P, nu1, nu2));
    }

    /// Returns the normal to the surface given an world space point which is presumed to
    /// be the collision point, thus on the surface.
    /// @param world_surface_point The point on the object to use.
    /// @warning It is assumed that the point given is an intersection point.
    /// @retval R3::null indicates that the point is not on the surface.
    virtual vector normal(point const& world_surface_point) const = 0;

    /// Returns all the intersections with this object along the ray (extended as a line).
    /// @param object_ray The ray in object space.
    /// @return The unordered set of intersection distances along the world ray.
    virtual hits collisions_along(ray const& object_ray) const = 0;

    /// Returns an intersection with this object
    /// @param world_ray The ray from the world space to test the intersection with.
    /// @return Returns the first intersection with the object.
    ///
    virtual intersection intersect(ray const& world_ray) const {
        /// @note While we could be pedantic about having a unit normal, it doesn't really stop us from working.
        // basal::exception::throw_unless(basal::nearly_equals(world_ray.direction().quadrance(), 1.0_p), __FILE__, __LINE__,
        // "The ray must have a unit vector");

        ray object_ray = entity_<DIMS>::reverse_transform(world_ray);
        // get the set of all collisions with the object
        hits ts = collisions_along(object_ray);
        // ts could contain NaN and +Inf/-Inf
        if (ts.size() > 0) {
            // determine the closest hit and transform the point
            precision closest = std::numeric_limits<precision>::infinity();
            for (auto t : ts) {
                if (basal::is_nan(t)) {
                    continue;
                }
                if constexpr (can_ray_origin_be_collision) {
                    if (basal::nearly_zero(t)) {
                        vector const N = normal(world_ray.location());
                        vector const& I = world_ray.direction();
                        precision d = dot(N, I);
                        if (d < 0) {  // the ray points "into" the material so it's a collision
                            // however if the material is transparent, that's ok and it should not count as a collision
                            if (m_medium and m_medium->refractive_index(world_ray.location()) > 0.0_p) {
                                continue;
                            }
                            closest = t;
                            break;
                        } else {
                            // the ray points "out" of the material so no collision
                        }
                    }
                }
                if (basal::epsilon < t and t < closest) {
                    closest = t;
                }
            }
            if (not std::isinf(closest)) {
                point object_point = object_ray.distance_along(closest);
                point world_point = entity_<DIMS>::forward_transform(object_point);
                statistics::get().intersections_with_objects++;
                return intersection(world_point);
            }
        }
        return intersection();
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

    /// Return true if the object is a closed surface.
    /// Within the raytracer a closed surface can never show the "inside surface" */
    virtual inline bool is_closed_surface() const {
        return m_closed_surface;
    }

    /// @brief Used to determine if an open surface and ray could ever possibly intersect.
    /// @param world_ray The ray in world space to test against.
    /// @return False if the ray is not along the infinite extent. Closed surfaces will always return false.
    virtual inline bool is_along_infinite_extent(ray const& world_ray) const {
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
        raytrace::point const&P = entity_<DIMS>::position();
        raytrace::vector V = world_point - P;
        P.print("center");
        world_point.print("world_point");
        V.print("projected_vector");
        N.print("world_normal");
        precision d = dot(N, V);
        printf("d=%lf\r\n", d);
        return (d > 0.0); // if the dot of the Normal on the point from center
    }

    /// Given a world point verify that the point is on the surface of the object.
    virtual bool is_surface_point(raytrace::point const& world_point) const {
        return false;  // override to correct
    };

    /// Computes the Axis Aligned Bounding Box in World Coordinates for this object.
    abba get_world_bounds(void) const {
        // finds the object's maximum radial distance from the object origin
        auto ijk_max = get_object_extent();
        // the 3d corner of a point with those components
        vector v{ijk_max, ijk_max, ijk_max};
        auto r = v.magnitude();
        // constructs a local trivial structure
        return abba{
            {entity_<DIMS>::forward_transform(point{-r, -r, -r}), entity_<DIMS>::forward_transform(point{r, r, r})}};
    }

    /// Returns the maximum radial distance from the object origin on the surface of the object.
    /// @warning Objects which are not closed or have infinite dimensionality will return std::nan
    virtual precision get_object_extent(void) const = 0;

    /// Allows setting the UV scaling factors independently.
    virtual void set_surface_scale(precision u, precision v) {
        m_surface_scale.u = u;
        m_surface_scale.v = v;
    }

protected:
    /// The maximum number of collisions with the surface of this object
    size_t const m_max_collisions;
    /// Some objects may return more than 1 collisions but are not closed surfaces
    bool const m_closed_surface;
    /// The pointer to the medium to use
    raytrace::mediums::medium const * m_medium;
    /// The std::bind or used to reference the instance of the mapping function
    std::function<geometry::R2::point(geometry::R3::point const&)> m_bound_map;
    /// The UV surface scaling factors
    struct {
            precision u; ///< The scaling factor for U
            precision v; ///< The scaling factor for V
    } m_surface_scale;
};

/// In Raytracing we only consider 3D objects
using object = object_<dimensions>;
}  // namespace objects
}  // namespace raytrace
