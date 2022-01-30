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
#include "raytrace/speck.hpp"
#include "raytrace/beam.hpp"
#include "raytrace/bulb.hpp"
#include "raytrace/spot.hpp"

// Mediums
#include "raytrace/medium.hpp"
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
#include "raytrace/cone.hpp"
#include "raytrace/cuboid.hpp"
#include "raytrace/cylinder.hpp"
#include "raytrace/overlap.hpp"
#include "raytrace/plane.hpp"
#include "raytrace/pyramid.hpp"
#include "raytrace/quadratic.hpp"
#include "raytrace/ring.hpp"
#include "raytrace/sphere.hpp"
#include "raytrace/square.hpp"
#include "raytrace/torus.hpp"
#include "raytrace/triangle.hpp"
