#pragma once

#include "raytrace/raytrace.hpp"

namespace raytrace {

/// The interface that demo_wrapper uses to construct all the objects then render
class world {
public:
    virtual raytrace::point& looking_from() = 0;
    virtual raytrace::point& looking_at() = 0;
    virtual std::string window_name() const = 0;
    virtual std::string output_filename() const = 0;
    virtual raytrace::color background(raytrace::ray const&) const = 0;
    /// Adds all the objects and lights to the scene.
    virtual void add_to(raytrace::scene& scene) = 0;
    virtual raytrace::animation::anchors get_anchors() const = 0;

protected:
    virtual ~world() = default;
};

/// A Function Type
using world_getter = world*();

}  // namespace raytrace

/// The interface to get a world from a module
extern "C" raytrace::world* get_world();
