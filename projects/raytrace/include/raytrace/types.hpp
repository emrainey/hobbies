#pragma once

#include <atomic>
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
    std::atomic<size_t> cast_rays_from_camera{0u};
    /// Intersections with objects
    std::atomic<size_t> intersections_with_objects{0u};
    /// Intersections with Single Point
    std::atomic<size_t> intersections_with_point{0u};
    /// Intersections with multiple points
    std::atomic<size_t> intersections_with_points{0u};
    /// Intersections with Lines
    std::atomic<size_t> intersections_with_line{0u};
    /// Intersections with bounding boxes
    std::atomic<size_t> intersections_with_bounds{0u};
    /// Intersection from the back side of an object (inside outwards or on the side away from the normal)
    std::atomic<size_t> inside_out_intersections{0u};
    /// No intersections with objects
    std::atomic<size_t> missed_rays{0u};
    /// The count of rays reflected off objects
    std::atomic<size_t> bounced_rays{0u};
    /// The count of rays transmitted through mediums via refraction
    std::atomic<size_t> transmitted_rays{0u};
    /// Saved Bounces from adaptive threshold
    std::atomic<size_t> saved_ray_traces{0u};
    /// The count of shadow rays used to determine lighting
    std::atomic<size_t> shadow_rays{0u};
    /// The count of rays added due to multiple samples from light sources
    std::atomic<size_t> sampled_rays{0u};
    /// The count of sampled rays which actually contribute to the color of the scene.
    std::atomic<size_t> color_sampled_rays{0u};
    /// The count of the points in the shadow which don't have a color contribution
    std::atomic<size_t> point_in_shadow{0u};
    /// The count of rays absorbed into a media
    /// this will not be an accurate count until the per-frequency method is done.
    std::atomic<size_t> absorbed_rays{0u};
    /// The number of rays emitted from light sources
    std::atomic<size_t> emitted_rays{0u};
    /// The number of tree nodes visited during intersection testing
    std::atomic<size_t> tree_nodes_visited{0u};
    /// The number of object intersection checks saved by pruning tree branches
    std::atomic<size_t> tree_checks_saved{0u};

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
