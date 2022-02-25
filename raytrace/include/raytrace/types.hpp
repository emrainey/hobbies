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
        /** The count of rays reflected off objects */
        size_t bounced_rays;
        /** The count of rays transmitted through mediums via refraction */
        size_t transmitted_rays;
        /** Saved Bounces from adaptive threshold */
        size_t saved_ray_traces;
        /** The count of shadow rays used to determine lighting */
        size_t shadow_rays;
        /** The count of rays added due to multiple samples from light sources */
        size_t sampled_rays;
        /** The count of sampled rays which actually contribute to the color of the scene. */
        size_t color_sampled_rays;
        /** The count of the points in the shadow which don't have a color contribution */
        size_t point_in_shadow;
        /** The count of rays absorbed into a media */
        size_t absorbed_rays;

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
            , saved_ray_traces(0)
            , shadow_rays(0)
            , sampled_rays(0)
            , color_sampled_rays(0)
            , point_in_shadow(0)
            , absorbed_rays(0)
            {}
    };
}
