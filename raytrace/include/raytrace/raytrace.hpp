#pragma once

#include <limits>
/** The Raytracing Namespace contains objects used for Raytracing */
namespace raytrace {
    // Enforce IEEE-754 compliance
    static_assert(std::numeric_limits<double>::is_iec559, "Platform must be IEEE-754 formatted.");
}

#include "raytrace/types.hpp"
#include "raytrace/scene.hpp"

// Lights
#include "raytrace/lights/speck.hpp"
#include "raytrace/lights/beam.hpp"
#include "raytrace/lights/bulb.hpp"
#include "raytrace/lights/spot.hpp"

// Mediums
#include "raytrace/mediums/medium.hpp"
#include "raytrace/mediums/opaque.hpp"
#include "raytrace/mediums/plain.hpp"
#include "raytrace/mediums/checkerboard.hpp"
#include "raytrace/mediums/grid.hpp"
#include "raytrace/mediums/dots.hpp"
#include "raytrace/mediums/marble.hpp"
#include "raytrace/mediums/random_noise.hpp"
#include "raytrace/mediums/transparent.hpp"
#include "raytrace/mediums/conductor.hpp"
#include "raytrace/mediums/metal.hpp"

// Objects
#include "raytrace/objects/cone.hpp"
#include "raytrace/objects/cuboid.hpp"
#include "raytrace/objects/cylinder.hpp"
#include "raytrace/objects/overlap.hpp"
#include "raytrace/objects/plane.hpp"
#include "raytrace/objects/pyramid.hpp"
#include "raytrace/objects/quadratic.hpp"
#include "raytrace/objects/paraboloid.hpp"
#include "raytrace/objects/hyperboloid.hpp"
#include "raytrace/objects/ellipsoid.hpp"
#include "raytrace/objects/ellipticalcone.hpp"
#include "raytrace/objects/ellipticalcylinder.hpp"
#include "raytrace/objects/ring.hpp"
#include "raytrace/objects/sphere.hpp"
#include "raytrace/objects/square.hpp"
#include "raytrace/objects/torus.hpp"
#include "raytrace/objects/triangle.hpp"
