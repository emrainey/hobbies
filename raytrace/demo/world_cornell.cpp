/**
 * @file
 * Simulation of the Cornell Box.
 * @author Erik Rainey (erik.rainey@gmail.com)
 * @copyright Copyright (c) 2021
 */

#include <raytrace/raytrace.hpp>
#include "world.hpp"

using namespace raytrace;

class cornell_box : public world {
public:
    cornell_box()
        : look_from(-220, 0, 80)
        , look_at(0, 0, 80)
        , steel(colors::grey, smoothness::polished, roughness::loose)
        , plain_white(colors::white, ambient::none, colors::white, smoothness::none, roughness::tight)
        , plain_red(colors::red, ambient::none, colors::red,  smoothness::none, roughness::tight)
        , plain_blue(colors::blue, ambient::none, colors::blue, smoothness::none, roughness::tight)
        , marble0(0.128283, 0.2, 32.0, colors::black, colors::yellow)
        , glass(refractive_index::glass, 0.9)
        , wall0(raytrace::point(0, 80, 80), -R3::basis::Y, 1.0) // left
        , wall1(raytrace::point(0, -80, 80), R3::basis::Y, 1.0) // right
        , wall2(raytrace::point(80, 0, 80), -R3::basis::X, 1.0) // back
        , wall3(R3::origin, R3::basis::Z, 1.0)                  // floor
        , wall4(raytrace::point(0, 0, 160), -R3::basis::Z, 1.0) // ceiling
        , box(raytrace::point(0, -30, 60), 20, 20, 60)
        , ball(raytrace::point(0, 30, 30), 30)
        , toplight(raytrace::point(0, 0, 150), colors::white, 1E11)
        {
        wall0.material(&plain_blue);
        wall1.material(&plain_red);
        wall2.material(&plain_white);
        wall3.material(&plain_white);
        wall4.material(&plain_white);
        box.rotation(iso::degrees(0), iso::degrees(0), iso::degrees(35));
        marble0.mapper(std::bind(&cuboid::map, &box, std::placeholders::_1)); // allows 2D mapping on the cube
        // box.material(&marble0);
        box.material(&glass);
        ball.material(&steel);
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

    raytrace::color background(const raytrace::ray&) const override {
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
        scene.add_light(&toplight);
    }
protected:
    raytrace::point look_from;
    raytrace::point look_at;
    metal steel;
    plain plain_white;
    plain plain_red;
    plain plain_blue;
    marble marble0;
    transparent glass;
    raytrace::plane wall0;
    raytrace::plane wall1;
    raytrace::plane wall2;
    raytrace::plane wall3;
    raytrace::plane wall4;
    cuboid box;
    raytrace::sphere ball;
    speck toplight;
};

// declare a single instance and return the reference to it
world* get_world() {
    static cornell_box my_world;
    return &my_world;
}
