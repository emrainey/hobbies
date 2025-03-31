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
        , look_from{0, -50, 10}
        , look_at{0, 5, 10}
        , light0{raytrace::point{0, 0, 10}, 1, colors::white, 1E11, light_subsamples}
        , speck0{raytrace::point{0, 0, 10}, colors::white, 1E2}
        , checkerboard_grid{0.25_p,        colors::blue, colors::yellow, colors::red,  colors::magenta,
                            colors::green, colors::cyan, colors::black,  colors::white}
        , floor{R3::origin, raytrace::vector{0, -1, 4}.normalized(), 20, 20}
        , ceiling{raytrace::point(0, 0, 20), raytrace::vector{0, -1, -4}.normalized(), 20, 20}
        , left{raytrace::point(-10, 0, 10), raytrace::vector{4, -1, 0}.normalized(), 20, 20}
        , right{raytrace::point(10, 0, 10), raytrace::vector{-4, -1, 0}.normalized(), 20, 20}
        , back{raytrace::point(0, 5, 10), -R3::basis::Y, 10, 10} {
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
        anchors.push_back(animation::Anchor{animation::Attributes{look_from, look_at, 75.0_deg},
                                            animation::Attributes{raytrace::point(15, -20, 10), look_at, 75.0_deg},
                                            animation::Mappers{}, iso::seconds{2.0_p}});
        anchors.push_back(animation::Anchor{anchors.back().limit,  // take last limit as start
                                            animation::Attributes{raytrace::point(-15, -20, 10), look_at, 75.0_deg},
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
