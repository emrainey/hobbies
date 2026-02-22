#pragma once

#include "raytrace/raytrace.hpp"

namespace raytrace {

/// The mixin that demo use to construct all the objects then render
class world {
public:
    /// Constructor
    world(raytrace::point const& from, raytrace::point const& at, std::string const& window_name,
          std::string const& file_name)
        : looking_from_(from)
        , looking_at_(at)
        , window_name_(window_name)
        , file_name_(file_name)
        , background_{colors::black}
        , ambient_{1.0_p, 1.0_p, 1.0_p, 0.75_p} {
    }

    /// Adds all the objects and lights to the scene.
    virtual void add_to(raytrace::scene& scene) = 0;

    /// Adds all the anchor points for the animation to the scene
    virtual raytrace::animation::anchors get_anchors() const = 0;

public:
    inline raytrace::point const& looking_from() const {
        return looking_from_;
    }

    inline raytrace::point& looking_from() {
        return looking_from_;
    }

    inline raytrace::point const& looking_at() const {
        return looking_at_;
    }

    inline raytrace::point& looking_at() {
        return looking_at_;
    }

    inline std::string window_name() const {
        return window_name_;
    }

    inline std::string output_filename() const {
        return file_name_;
    }

    virtual color background(raytrace::ray const&) const {
        return background_;
    }

    virtual color ambient() const {
        // this carries intensity in the 4th channel!
        return ambient_;
    }

protected:
    raytrace::point looking_from_;
    raytrace::point looking_at_;
    std::string window_name_;
    std::string file_name_;
    raytrace::color background_;
    raytrace::color ambient_;

    virtual ~world() = default;
};

/// A Function Type
using world_getter = world*();

}  // namespace raytrace

/// The interface to get a world from a module
extern "C" raytrace::world* get_world();
