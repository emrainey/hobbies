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
        : world{raytrace::point{0, -50, 10}, raytrace::point{0, 5, 10}, "Mirror Scene", "world_mirrors.tga"}
        , light_subsamples{20}
        , light0{raytrace::point{0, 0, 10}, 1, colors::white, 1E11, light_subsamples}
        , speck0{raytrace::point{0, 0, 10}, colors::white, 1E2}
        , checkerboard_grid{0.25_p,        colors::blue, colors::yellow, colors::red,  colors::magenta,
                            colors::green, colors::cyan, colors::black,  colors::white}
        , a{tan(0.25) / iso::tau}
        , floor{R3::origin, R3::roll(a), 40}
        , ceiling{raytrace::point(0, 0, 20), R3::roll(0.5 - a), 40}
        , left{raytrace::point(-10, 0, 10), R3::yaw(-a) * R3::pitch(0.25), 40}
        , right{raytrace::point(10, 0, 10), R3::yaw(a) * R3::pitch(-0.25), 40}
        , back{raytrace::point(0, 5, 10), R3::roll(0.25), 20} {
        // assign surfaces and materials
        floor.material(&metals::silver);
        ceiling.material(&metals::silver);
        left.material(&metals::silver);
        right.material(&metals::silver);
        checkerboard_grid.mapper(std::bind(&objects::square::map, &back, std::placeholders::_1));
        back.material(&checkerboard_grid);
    }

    ~MirrorWorld() = default;

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
        anchors.push_back(animation::Anchor{animation::Attributes{looking_from(), looking_at(), 75.0_deg},
                                            animation::Attributes{raytrace::point(15, -20, 10), looking_at(), 75.0_deg},
                                            animation::Mappers{}, iso::seconds{2.0_p}});
        anchors.push_back(
            animation::Anchor{anchors.back().limit,  // take last limit as start
                              animation::Attributes{raytrace::point(-15, -20, 10), looking_at(), 75.0_deg},
                              animation::Mappers{}, iso::seconds{2.0_p}});
        return anchors;
    }

protected:
    size_t light_subsamples;
    raytrace::lights::bulb light0;
    raytrace::lights::speck speck0;
    mediums::checkerboard checkerboard_grid;
    precision a;
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
