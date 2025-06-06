///
/// @file
/// Simulation of the Cornell Box.
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @copyright Copyright (c) 2021
///

#include <raytrace/raytrace.hpp>

#include "world.hpp"

using namespace raytrace;
using namespace iso::literals;

class cornell_box : public world {
public:
    cornell_box()
        : look_from{-220, 0, 80}
        , look_at{0, 0, 80}
        , plain_white(colors::white, mediums::ambient::none, colors::white, mediums::smoothness::none,
                      mediums::roughness::tight)
        , plain_red(colors::red, mediums::ambient::none, colors::red, mediums::smoothness::none,
                    mediums::roughness::tight)
        , plain_blue(colors::blue, mediums::ambient::none, colors::blue, mediums::smoothness::none,
                     mediums::roughness::tight)
        , marble0{0.128283_p, 0.2_p, 32.0_p, colors::black, colors::yellow}
        , glass{mediums::refractive_index::glass, 0.02_p, colors::gray}
        , wall0{raytrace::point{0, 80, 80}, R3::roll(iso::radians{iso::pi / 2})}    // left
        , wall1{raytrace::point{0, -80, 80}, R3::roll(iso::radians{-iso::pi / 2})}  // right
        , wall2{raytrace::point{80, 0, 80}, R3::pitch(iso::radians{-iso::pi / 2})}  // back
        , wall3{}                                                                   // floor
        , wall4{raytrace::point{0, 0, 160}, R3::pitch(iso::radians{iso::pi})}       // ceiling
        , box{raytrace::point{0, -30, 60}, 20, 20, 60}
        , ball{raytrace::point{0, 30, 30}, 30}
        , top_light{raytrace::point{0, 0, 150}, colors::white, 1E4} {
        wall0.material(&plain_blue);
        wall1.material(&plain_red);
        wall2.material(&plain_white);
        wall3.material(&plain_white);
        wall4.material(&plain_white);
        box.rotation(iso::degrees(0), iso::degrees(0), iso::degrees(35));
        marble0.mapper(std::bind(&objects::cuboid::map, &box, std::placeholders::_1));  // allows 2D mapping on the cube
        // box.material(&marble0);
        box.material(&glass);
        ball.material(&mediums::metals::stainless);
    }

    raytrace::point& looking_from() override {
        return look_from;
    }

    raytrace::point& looking_at() override {
        return look_at;
    }

    std::string window_name() const override {
        return std::string("Cornell Box -ish");
    }

    std::string output_filename() const override {
        return std::string("world_cornell.tga");
    }

    color background(raytrace::ray const&) const override {
        return colors::black;
    }

    void add_to(scene& scene) override {
        // add the objects to the scene.
        scene.add_object(&wall0);
        scene.add_object(&wall1);
        scene.add_object(&wall2);
        scene.add_object(&wall3);
        scene.add_object(&wall4);
        scene.add_object(&box);
        scene.add_object(&ball);
        scene.add_light(&top_light);
    }

    raytrace::animation::anchors get_anchors() const override {
        raytrace::animation::anchors anchors;
        anchors.push_back(animation::Anchor{animation::Attributes{look_from, look_at, 55.0_deg},
                                            animation::Attributes{look_from, look_at, 55.0_deg}, animation::Mappers{},
                                            iso::seconds{1.0_p}});
        return anchors;
    }

protected:
    raytrace::point look_from;
    raytrace::point look_at;
    mediums::plain plain_white;
    mediums::plain plain_red;
    mediums::plain plain_blue;
    mediums::perlin marble0;
    mediums::transparent glass;
    objects::plane wall0;
    objects::plane wall1;
    objects::plane wall2;
    objects::plane wall3;
    objects::plane wall4;
    objects::cuboid box;
    objects::sphere ball;
    lights::speck top_light;
};

// declare a single instance and return the reference to it
world* get_world() {
    static cornell_box my_world;
    return &my_world;
}
