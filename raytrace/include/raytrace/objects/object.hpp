#pragma once

#include "basal/basal.hpp"
#include "raytrace/entity.hpp"
#include "raytrace/laws.hpp"
#include "raytrace/mediums/medium.hpp"
#include "raytrace/mediums/plain.hpp"

namespace raytrace {
/** The namespace to contain all (spatial) objects */
namespace objects {

/** A flag to control if origin collisions are counted */
constexpr bool can_ray_origin_be_collision = true;

using namespace linalg;
using namespace geometry;
using namespace raytrace::mediums;

/** A template for wrapping the concept of an object, which can have a surface and material properties. */
template <size_t DIMS>
class object_
    : public entity_<DIMS>
    , public basal::printable {
public:
    object_() : entity_<DIMS>(), m_max_collisions{0}, m_closed_surface{false}, m_medium(&mediums::dull) {
    }

    object_(const point& center, size_t collisions, bool closed = false)
        : entity_<DIMS>(center)
        , m_max_collisions{collisions}
        , m_closed_surface{collisions > 1 ? closed : false}
        , m_medium{&mediums::dull} {
    }

    object_(point&& center, size_t collisions, bool closed = false)
        : entity_<DIMS>(std::move(center))
        , m_max_collisions{collisions}
        , m_closed_surface{collisions > 1 ? closed : false}
        , m_medium{&mediums::dull} {
    }

    // Removing copy constructor stops us from using std::vectors or other STL objects
    object_(const object_&) = delete;
    object_(object_&&) = delete;
    object_& operator=(const object_&) = delete;
    object_& operator=(object_&&) = delete;

    virtual ~object_() {
        m_medium = nullptr;
    }

    /** Gives a read-only version of the medium of the object */
    const medium& material() const {
        basal::exception::throw_if(m_medium == nullptr, __FILE__, __LINE__, "Can not request a nullptr.");
        return *m_medium;
    }

    /** Sets the material to the given parameter medium */
    virtual void material(const medium* new_medium) {
        basal::exception::throw_if(new_medium == nullptr, __FILE__, __LINE__, "Can not set a nullptr to material()");
        m_medium = new_medium;
    }

    /** Computes the reflection vector at the surface point P given the incident vector I */
    vector reflection(const vector& I, const point& P) const {
        vector N = normal(P);  // MAYBE (Optim) should we already have calculated the normal at this point already?
        vector nI = I.normalized();  // MAYBE (Optim) shouldn't this already be normalized?
        vector R = laws::reflection(N, nI);
        if (m_medium) {
            R = R + m_medium->perturbation(P);
        }
        return R;
    }

    /** Computes the reflection ray at the surface point P given the incident ray I */
    ray reflection(const ray& I, const point& P) const {
        return ray(P, reflection(I.direction(), P));
    }

    /** Computes the refraction vector at the surface point
     * @param I The incident vector
     * @param P The surface point
     * @param nu1 Material Refractivity Index
     * @param nu2 Material Refractivity Index
     */
    vector refraction(const vector& I, const point& P, element_type nu1, element_type nu2) const {
        vector N = normal(P);  // MAYBE (Optim) should we already have calculated the normal at this point already?
        vector nI = I.normalized();  // MAYBE (Optim) shouldn't this already be normalized?
        // if the Incident and the Normal have a positive dot then they are not arranged properly
        if (dot(N, I) > 0) {
            return laws::snell(-N, nI, nu1, nu2);
        } else {
            return laws::snell(N, nI, nu1, nu2);
        }
    }

    /** Computes the refraction ray at the surface point P.
     * @param I The incident ray
     * @param P The surface point
     * @param nu1 Material Refractivity Index
     * @param nu2 Material Refractivity Index
     */
    ray refraction(const ray& I, const point& P, element_type nu1, element_type nu2) const {
        return ray(P, refraction(I.direction(), P, nu1, nu2));
    }

    /**
     * Returns the normal to the surface given an world space point which is presumed to be the collision point, thus on
     * the surface.
     * @param world_surface_point The point on the object to use.
     * @warning It is assumed that the point given is an intersection point.
     * @retval R3::null indicates that the point is not on the surface.
     */
    virtual vector normal(const point& world_surface_point) const = 0;

    /**
     * Returns all the intersections with this object along the ray (extended as a line).
     * @param object_ray The ray in object space.
     * @return The unordered set of intersection distances along the world ray.
     */
    virtual hits collisions_along(const ray& object_ray) const = 0;

    /** Returns an intersection with this object
     * @param world_ray The ray from the world space to test the intersection with.
     * @return Returns the first intersection with the object.
     */
    virtual intersection intersect(const ray& world_ray) const {
        /// @note While we could be pedantic about having a unit normal, it doesn't really stop us from working.
        // basal::exception::throw_unless(basal::equals(world_ray.direction().quadrance(), 1.0), __FILE__, __LINE__,
        // "The ray must have a unit vector");

        ray object_ray = entity_<DIMS>::reverse_transform(world_ray);
        // get the set of all collisions with the object
        hits ts = collisions_along(object_ray);
        // ts could contain NaN and +Inf/-Inf
        if (ts.size() > 0) {
            // determine the closest hit and transform the point
            element_type closest = std::numeric_limits<element_type>::infinity();
            for (auto t : ts) {
                if (std::isnan(t)) {
                    continue;
                }
                if constexpr (can_ray_origin_be_collision) {
                    if (basal::equals_zero(t)) {
                        const vector N = normal(world_ray.location());
                        const vector& I = world_ray.direction();
                        element_type d = dot(N, I);
                        if (d < 0) {  // the ray points "into" the material so it's a collision
                            // however if the material is transparent, that's ok and it should not count as a collision
                            if (m_medium and m_medium->refractive_index(world_ray.location()) > 0.0) {
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

    /**
     * Maps a surface point (in R3 object space) to a image::point in u,v coordinates (R2)
     * @param object_surface_point The point on the surface of the object in object space coordinates
     */
    virtual image::point map(const point& object_surface_point) const = 0;

    /** Returns the maximum number of surface collisions that the object can return.
     * It is assumed that the actual collisions could be between 0 and this number.
     */
    virtual inline size_t max_collisions() const {
        return m_max_collisions;
    }

    /** Return true if the object is a closed surface.
     * Within the raytracer a closed surface can never show the "inside surface" */
    virtual inline bool is_closed_surface() const {
        return m_closed_surface;
    }

    /** Given a world point verify that the point is on the surface of the object. */
    virtual bool is_surface_point(const raytrace::point& world_point) const {
        return false;  // override to correct
    };

protected:
    /** The maximum number of collisions with the surface of this object */
    const size_t m_max_collisions;
    /** Some objects may return more than 1 collisions but are not closed surfaces */
    const bool m_closed_surface;
    /** The pointer to the medium to use */
    const raytrace::mediums::medium* m_medium;
    /** The std::bind or used to reference the instance of the mapping function */
    std::function<geometry::R2::point(const geometry::R3::point&)> m_bound_map;
};

/** In Raytracing we only consider 3D objects */
using object = object_<dimensions>;
}  // namespace objects
}  // namespace raytrace
