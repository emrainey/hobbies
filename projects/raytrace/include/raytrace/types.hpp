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

/// A counter with relaxed memory ordering.
/// Statistics are only read at the end of rendering (after all OpenMP threads join),
/// so we never need sequential consistency — relaxed is correct and avoids barrier overhead.
struct relaxed_counter {
    std::atomic<size_t> value{0u};
    relaxed_counter& operator+=(size_t n) {
        value.fetch_add(n, std::memory_order_relaxed);
        return *this;
    }
    size_t operator++(int) {
        return value.fetch_add(1, std::memory_order_relaxed);
    }
    size_t load() const {
        return value.load(std::memory_order_relaxed);
    }
    operator size_t() const {
        return load();
    }
};

/// Collects the statistics from the raytracing library
struct statistics {
public:
    /// The number of rays cast from the camera
    relaxed_counter cast_rays_from_camera;
    /// Intersections with objects
    relaxed_counter intersections_with_objects;
    /// Intersections with Single Point
    relaxed_counter intersections_with_point;
    /// Intersections with multiple points
    relaxed_counter intersections_with_points;
    /// Intersections with Lines
    relaxed_counter intersections_with_line;
    /// Intersections with bounding boxes
    relaxed_counter intersections_with_bounds;
    /// Intersection from the back side of an object (inside outwards or on the side away from the normal)
    relaxed_counter inside_out_intersections;
    /// No intersections with objects
    relaxed_counter missed_rays;
    /// The count of rays reflected off objects
    relaxed_counter bounced_rays;
    /// The count of rays transmitted through mediums via refraction
    relaxed_counter transmitted_rays;
    /// Saved Bounces from adaptive threshold
    relaxed_counter saved_ray_traces;
    /// The count of shadow rays used to determine lighting
    relaxed_counter shadow_rays;
    /// The count of rays added due to multiple samples from light sources
    relaxed_counter sampled_rays;
    /// The count of sampled rays which actually contribute to the color of the scene.
    relaxed_counter color_sampled_rays;
    /// The count of the points in the shadow which don't have a color contribution
    relaxed_counter point_in_shadow;
    /// The count of rays absorbed into a media
    /// this will not be an accurate count until the per-frequency method is done.
    relaxed_counter absorbed_rays;
    /// The number of rays emitted from light sources
    relaxed_counter emitted_rays;
    /// The number of tree nodes visited during intersection testing
    relaxed_counter tree_nodes_visited;
    /// The number of object intersection checks saved by pruning tree branches
    relaxed_counter tree_checks_saved;

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
