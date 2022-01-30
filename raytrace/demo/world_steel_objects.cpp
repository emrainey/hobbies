/**
 * @file
 * The Steel Object Scene from year ago with Vivid 3.
 * @author Erik Rainey (erik.rainey@gmail.com)
 * @copyright Copyright (c) 2021
 */

#include <raytrace/raytrace.hpp>
#include <basal/options.hpp>
#include "world.hpp"

using namespace raytrace;

class SteelWorld : public world {
public:
    SteelWorld()
        : look_from(5, -20, 20)
        , look_at(0, 0, 0)
        , steel(colors::grey, smoothness::polished, roughness::loose)
        , plain_white(colors::white, ambient::none, colors::white, smoothness::none, roughness::tight)
        , ground(raytrace::origin, R3::basis::Z, 1)
        , light0(raytrace::point(-5, 0, 10), 1, colors::white, 10, 20)
        , light1(raytrace::point(-4, 0, 10), 1, colors::white, 10, 20)
        , light2(raytrace::point(-3, 0, 10), 1, colors::white, 10, 20)
        , light3(raytrace::point(-2, 0, 10), 1, colors::white, 10, 20)
        , light4(raytrace::point(-1, 0, 10), 1, colors::white, 10, 20)
        , light5(raytrace::point( 0, 0, 10), 1, colors::white, 10, 20)
        , light6(raytrace::point( 1, 0, 10), 1, colors::white, 10, 20)
        , light7(raytrace::point( 2, 0, 10), 1, colors::white, 10, 20)
        , light8(raytrace::point( 3, 0, 10), 1, colors::white, 10, 20)
        , light9(raytrace::point( 4, 0, 10), 1, colors::white, 10, 20)
        , light10(raytrace::point(5, 0, 10), 1, colors::white, 10, 20)
        , s1(raytrace::point(4, 2, 1), 1)
        , s2(raytrace::point(-4,-2, 2), 2)
        , s3(raytrace::point(1, -5, 3), 3)
        , c1(raytrace::point(6, 3, 0), 1, 4) // cone
        , cyl(raytrace::point(-2, 3, 2), 2, 1) // cylinder
        , cap(raytrace::point(-2, 3, 4), R3::basis::Z, 0, 1)
        {
            // assign surfaces and materials
            ground.material(&plain_white);
            s1.material(&steel);
            s2.material(&steel);
            s3.material(&steel);
            c1.material(&steel);
            cyl.material(&steel);
            cap.material(&steel);
    }

    raytrace::point& looking_from() override {
        return look_from;
    }

    raytrace::point& looking_at() override {
        return look_at;
    }

    std::string window_name() const override {
        return std::string("Steel World");
    }

    std::string output_filename() const override {
        return std::string("world_steel.ppm");
    }

    raytrace::color background(const raytrace::ray& world_ray) const override {
        iso::radians A = angle(R3::basis::Z, world_ray.direction());
        element_type B = A.value / iso::pi;
        return color(0.8 * B, 0.8 * B, 0.8 * B);
    }

    void add_to(scene& scene) override {
        // add lights to the scene
        scene.add_light(&light0);
        scene.add_light(&light1);
        scene.add_light(&light2);
        scene.add_light(&light3);
        scene.add_light(&light4);
        scene.add_light(&light5);
        scene.add_light(&light6);
        scene.add_light(&light7);
        scene.add_light(&light8);
        scene.add_light(&light9);
        scene.add_light(&light10);
        // add the objects to the scene.
        scene.add_object(&ground);
        scene.add_object(&s1);
        scene.add_object(&s2);
        scene.add_object(&s3);
        scene.add_object(&c1);
        scene.add_object(&cyl);
        scene.add_object(&cap);
    }
protected:
    raytrace::point look_from;
    raytrace::point look_at;
    metal steel;
    plain plain_white;
    raytrace::plane ground;
    bulb light0;
    bulb light1;
    bulb light2;
    bulb light3;
    bulb light4;
    bulb light5;
    bulb light6;
    bulb light7;
    bulb light8;
    bulb light9;
    bulb light10;
    raytrace::sphere s1;
    raytrace::sphere s2;
    raytrace::sphere s3;
    cone c1;
    cylinder cyl;
    ring cap;
};

// declare a single instance and return the reference to it
world* get_world() {
    static SteelWorld my_world;
    return &my_world;
}
