#pragma once

#include <geometry/geometry.hpp>
//#include "xmmt.hpp"

namespace raytrace {
    using namespace geometry::operators;
    using namespace geometry::R3; // only 3D functions
    constexpr const static size_t dimensions = geometry::R3::dimensions;
    /** Reuse the element type */
    using element_type = geometry::element_type;
    /** We only consider 3D points in space */
    using point = geometry::R3::point;
    /** We only consider 3D vectors */
    using vector = geometry::R3::vector;
    /** We only consider 3D rays */
    using ray = geometry::R3::ray;
    /** We consider only 3D lines */
    using line = geometry::R3::line;
    /** Reusing other matrix */
    using matrix = linalg::matrix;
    /** A hit is both a distance along the line and the normal at that point */
    struct hit {
        element_type t; //!< The distance along the line.
        vector       N; //!< The normal at the point along the line.
    };
    /** A set of distances along the world_ray which collide with the object, could be many. */
    using hits = std::vector<element_type>;

    /** Collects the statistics from the raytracing library */
    struct statistics {
    public:
        /** The number of rays cast from the camera */
        size_t cast_rays_from_camera;
        /** Intersections with objects */
        size_t intersections_with_objects;
        /** Bounces from reflections */
        size_t bounced_rays;
        /** Transmitted Rays */
        size_t transmitted_rays;

        static statistics& get() {
            static statistics s;
            return s;
        }
    protected:
        constexpr statistics()
            : cast_rays_from_camera(0)
            , intersections_with_objects(0)
            , bounced_rays(0)
            , transmitted_rays(0)
            {}
    };
}
