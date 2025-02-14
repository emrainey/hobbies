#pragma once

#include <geometry/geometry.hpp>

namespace raytrace {
using namespace geometry::operators;
using namespace geometry::R3;  // only 3D functions
constexpr static const size_t dimensions = geometry::R3::dimensions;
/// Reuse the element type
using precision = geometry::precision;
/// We only consider 3D points in space
using point = geometry::R3::point;
/// We only consider 3D vectors
using vector = geometry::R3::vector;
/// We only consider 3D rays
using ray = geometry::R3::ray;
/// We consider only 3D lines
using line = geometry::R3::line;
/// Reusing other matrix
using matrix = linalg::matrix;

/// Determines if points are on surfaces, thus having a normal
constexpr static bool check_on_surface = geometry::check_on_surface;
/// An axis aligned bounding box for computing the BSP
struct abba {
    point min;  //!< The lower set of values.
    point max;  //!< The higher set of values.
};

/// Collects the statistics from the raytracing library
struct statistics {
public:
    /// The number of rays cast from the camera
    size_t cast_rays_from_camera{0u};
    /// Intersections with objects
    size_t intersections_with_objects{0u};
    /// Intersections with Single Point
    size_t intersections_with_point{0u};
    /// Intersections with multiple points
    size_t intersections_with_points{0u};
    /// Intersections with Lines
    size_t intersections_with_line{0u};
    /// The count of rays reflected off objects
    size_t bounced_rays{0u};
    /// The count of rays transmitted through mediums via refraction
    size_t transmitted_rays{0u};
    /// Saved Bounces from adaptive threshold
    size_t saved_ray_traces{0u};
    /// The count of shadow rays used to determine lighting
    size_t shadow_rays{0u};
    /// The count of rays added due to multiple samples from light sources
    size_t sampled_rays{0u};
    /// The count of sampled rays which actually contribute to the color of the scene.
    size_t color_sampled_rays{0u};
    /// The count of the points in the shadow which don't have a color contribution
    size_t point_in_shadow{0u};
    /// The count of rays absorbed into a media
    size_t absorbed_rays{0u};  // FIXME this will not accurately count until a per frequency method done.

    static statistics& get() {
        static statistics s;
        return s;
    }
};
}  // namespace raytrace
