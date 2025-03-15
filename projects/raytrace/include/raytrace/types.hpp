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
    /// Intersection from the back side of an object (inside outwards or on the side away from the normal)
    size_t inside_out_intersections{0u};
    /// No intersections with objects
    size_t missed_rays{0u};
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

/// Computes the values of a convex lens based on the diameter and angle of the lens.
/// Once initialized use the values to construct two sphere at a +/- separation from the origin of the specific radius.
/// Pass both into an inclusive overlap to make a convex lens.
class ConvexLensData {
public:
    /// Constructs a convex lens with the given diameter and angle.
    /// @param lens_diameter The desired diameter of the lens itself, not the intersecting spheres.
    /// @param half_angle The half angle of the lens in radians.
    ConvexLensData(precision lens_diameter, iso::radians half_angle) {
        radius = (lens_diameter * 0.5) / std::sin(half_angle.value);
        separation = (lens_diameter * 0.5) / std::tan(half_angle.value);
        bulge = radius - separation;
    }
    precision radius;      ///< The radius of the sphere needed
    precision separation;  ///< from the common origin to the center of the overlapped spheres, not the absolute
                           ///< distance between their points
    precision bulge;       ///< the thickness of the lens from the origin to the edge of the glass
};

}  // namespace raytrace
