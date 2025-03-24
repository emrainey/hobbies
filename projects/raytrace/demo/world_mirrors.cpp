///
/// @file
/// The Mirrors World Scene.
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @copyright Copyright (c) 2021
///

#include <functional>
#include <raytrace/raytrace.hpp>

#include "world.hpp"

using namespace raytrace;
using namespace raytrace::objects;
using namespace raytrace::lights;
using namespace iso::literals;

class MirrorWorld : public world {
public:
    MirrorWorld()
        : light_subsamples{20}
        , look_from{-285, -705, 200}
        , look_at{0, 50, 200}
        , light0{raytrace::point{0, 0, 100}, 1, colors::white, 1E11, light_subsamples}
        , speck0{raytrace::point{0, 0, 100}, colors::white, 1E5}
        , checkerboard_grid{0.1_p,         colors::blue, colors::yellow, colors::red,  colors::magenta,
                            colors::green, colors::cyan, colors::black,  colors::white}
        , floor{R3::origin, raytrace::vector{0, -1, 2}.normalized(), 100, 100}
        , ceiling{raytrace::point(0, 0, 200), raytrace::vector{0, -1, -2}.normalized(), 100, 100}
        , left{raytrace::point(-100, 0, 100), raytrace::vector{2, -1, 0}.normalized(), 100, 100}
        , right{raytrace::point(100, 0, 100), raytrace::vector{-2, -1, 0}.normalized(), 100, 100}
        , back{raytrace::point(0, 50, 100), raytrace::vector{0, -1, 0}.normalized(), 100, 100} {
        // assign surfaces and materials
        floor.material(&metals::silver);
        ceiling.material(&metals::silver);
        left.material(&metals::silver);
        right.material(&metals::silver);
        checkerboard_grid.mapper(std::bind(&objects::square::map, &back, std::placeholders::_1));
        back.material(&checkerboard_grid);
    }

    raytrace::point& looking_from() override {
        return look_from;
    }

    raytrace::point& looking_at() override {
        return look_at;
    }

    std::string window_name() const override {
        return std::string("Mirror Scene");
    }

    std::string output_filename() const override {
        return std::string("world_mirrors.tga");
    }

    raytrace::color background(raytrace::ray const& world_ray) const override {
        // this creates a gradient from top to bottom
        iso::radians A = angle(R3::basis::Z, world_ray.direction());
        precision B = A.value / iso::pi;
        return color(0.8_p * B, 0.8_p * B, 0.8_p * B);
    }

    void add_to(scene& scene) override {
        // add lights to the scene
        // scene.add_light(&light0);
        scene.add_light(&speck0);
        // add the objects to the scene.
        scene.add_object(&floor);
        scene.add_object(&ceiling);
        scene.add_object(&left);
        scene.add_object(&right);
        scene.add_object(&back);
    }

    raytrace::animation::anchors get_anchors() const override {
        raytrace::animation::anchors anchors;
        anchors.push_back(animation::Anchor{animation::Attributes{look_from, look_at, 55.0_deg},
                                            animation::Attributes{raytrace::point(0, 0, 50), look_at, 23.0_deg},
                                            animation::Mappers{}, iso::seconds{2.0_p}});
        return anchors;
    }

protected:
    size_t light_subsamples;
    raytrace::point look_from;
    raytrace::point look_at;
    raytrace::lights::bulb light0;
    raytrace::lights::speck speck0;
    mediums::checkerboard checkerboard_grid;
    raytrace::objects::square floor;
    raytrace::objects::square ceiling;
    raytrace::objects::square left;
    raytrace::objects::square right;
    raytrace::objects::square back;
};

// declare a single instance and return the reference to it
world* get_world() {
    static MirrorWorld my_world;
    return &my_world;
}
