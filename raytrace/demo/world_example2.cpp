/**
 * @file
 * @author Erik Rainey (erik.rainey@gmail.com)
 * @brief Renders an example raytraced image.
 * @date 2020-05-24
 * @copyright Copyright (c) 2020
 */

#include <raytrace/raytrace.hpp>
#include "world.hpp"

using namespace raytrace;

class Example2World : public world {
public:
    Example2World()
        : world()
        , look_from(20, 20, 10)
        , look_at(0, 0, 0)
        , glass_ball(raytrace::point(0, 0, 5.0), 5.0)
        //, glass_cube(raytrace::point(0, 10, 2), 2, 2, 2)
        , toy_ball(raytrace::point(-20, -20, 2.0), 2.0)
        , floor(R3::origin, R3::basis::Z, 100, 100)
        , ikea_checkers(0.1, colors::blue, colors::yellow, colors::red, colors::magenta, colors::green, colors::cyan, colors::black, colors::white)
        , schott_glass(mediums::refractive_index::glass, 0.3)
        , red_plastic(colors::white, mediums::ambient::dim, colors::red, mediums::smoothness::barely, mediums::roughness::tight)
        , sunlight(raytrace::vector{-2, 2, -1}, colors::white, 1E11)
        , backlight(raytrace::point(-10, -10, 12), colors::white, 1E3)
        , frontlight(raytrace::point(10, 10, 12), colors::white, 1E3)
        {
            ikea_checkers.mapper(std::bind(&objects::square::map, &floor, std::placeholders::_1));
            floor.material(&ikea_checkers);
            glass_ball.material(&schott_glass);
            //glass_cube.material(&schott_glass);
            //glass_cube.rotation(iso::radians(0), iso::radians(0), iso::radians(iso::pi/5));
            toy_ball.material(&red_plastic);
        }

    ~Example2World() = default;

    raytrace::point& looking_from() override {
        return look_from;
    }

    raytrace::point& looking_at() override {
        return look_at;
    }

    std::string window_name() const override {
        return std::string("Raytracing Example 2");
    }

    std::string output_filename() const override {
        return std::string("world_example2.tga");
    }

    raytrace::color background(const raytrace::ray&) const override {
        return colors::black;
    }

    void add_to(scene& scene) override {
        scene.add_object(&glass_ball);
        //scene.add_object(&glass_cube);
        scene.add_object(&floor);
        scene.add_object(&toy_ball);
        scene.add_light(&sunlight);
        scene.add_light(&backlight);
        scene.add_light(&frontlight);
    }

protected:
    raytrace::point look_from;
    raytrace::point look_at;
    objects::sphere glass_ball;
    //objects::cuboid glass_cube;
    objects::sphere toy_ball;
    objects::square floor;
    mediums::checkerboard ikea_checkers;
    mediums::transparent schott_glass;
    mediums::plain red_plastic;
    lights::beam sunlight;
    lights::speck backlight;
    lights::speck frontlight;
};

// declare a single instance and return the reference to it
world* get_world() {
    static Example2World my_world;
    return &my_world;
}
