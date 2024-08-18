///
/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Renders an example raytraced image.
/// @date 2020-05-24
/// @copyright Copyright (c) 2020
///

#include <raytrace/raytrace.hpp>

#include "world.hpp"

using namespace raytrace;
using namespace raytrace::objects;
using namespace raytrace::lights;

class ExampleWorld : public world {
public:
    ExampleWorld()
        : world{}
        // , look_from{0, 0, 60}
        , look_from{-50, -50, 100}
        , look_at{60, 60, 0}
        // , look_at{0, 0, 0}
        , tilt{{-0.2_p, +0.2_p, 1.0_p}}
        // define some surfaces
        , plain_green{colors::green, mediums::ambient::none, colors::green, 0.2_p, 100.0_p}
        , plain_yellow{colors::yellow, mediums::ambient::none, colors::yellow, 0.2_p, 100.0_p}
        , plain_blue{colors::white, mediums::ambient::none, colors::blue, 0.2_p, 100.0_p}
        , plain_red{colors::white, mediums::ambient::none, colors::red, 0.2_p, 100.0_p}
        , plain_cyan{colors::white, mediums::ambient::none, colors::cyan, 0.2_p, 120.0_p}
        , grey_checkers{3, colors::grey, colors::red}
        , ikea_checkers{5, colors::yellow, colors::blue}
        , polka_dots{3.0_p, colors::black, colors::white}
        , bw_marble{128.0_p, 128.0_p, 1.0_p, 1.0_p/1024.0_p, colors::black, colors::white}
        , grid1{10.0_p, colors::green, colors::black}
        , s2r{7.5_p}
        , r0r1{s2r + 2.0_p}
        , r0r2{r0r1 + 9.0_p}
        , pos0{60, 60, s2r}
        , s2{pos0, s2r}
        , ring0{pos0, tilt, r0r1, r0r2}
        , ring1{pos0, -tilt, r0r1, r0r2}
        , pos1{20, 80, 10}
        , top{pos1 + R3::vector{{0, 0, 10}}}
        , cyl1{pos1, 10, 10}
        , cap{top, R3::basis::Z, 0, 10}
        , po2{80, 20, 7.5_p}
        , cube0{po2, 7.5_p, 7.5_p, 7.5_p}
        , ground{R3::origin, R3::basis::Z}
        , floor{R3::origin, R3::basis::Z, 1000.0, 1000.0}
        , pos5{80, 80, 30}
        , pos6{90, 70, 0}
        , pos7{70, 90, 0}
        , tri0{pos5, pos6, pos7}
        , tri1{pos7, pos6, pos5}
        // , pos8{60, 30, 20}
        // , cone1(pos8, 7, 20)
        , pos8{60, 30, 10}
        , cone1{pos8, iso::radians(iso::pi / 12)}
        , bounding_cone{raytrace::point{60, 30, 10}, 10, 10, 10}
        , cone2{cone1, bounding_cone, overlap::type::inclusive}
        , pos9{30, 50, 10}
        , torus1{pos9, 7, 2}
        , blue_light{raytrace::point{80, 40, 40}, colors::blue, lights::intensities::moderate}
        , red_light{raytrace::point{40, 80, 40}, colors::red, lights::intensities::moderate}
        , green_light{raytrace::point{40, 40, 40}, colors::green, lights::intensities::moderate}
        , white_light{raytrace::point{20, 20, 20}, colors::white, 1E2}
        , sunlight{raytrace::vector{-2, 2, -1}, colors::white, lights::intensities::full} {
        // assign materials in the body of the constructor!

        // 2d mapping instead of 3d
        ikea_checkers.mapper(std::bind(&raytrace::objects::sphere::map, &s2, std::placeholders::_1));
        s2.material(&ikea_checkers);
        // 2d mapping instead of 3d
        ring0.material(&plain_blue);
        ring1.material(&plain_blue);
        // 2d mapping instead of 3d
        bw_marble.mapper(std::bind(&raytrace::objects::cylinder::map, &cyl1, std::placeholders::_1));
        cyl1.material(&bw_marble);
        // 2d mapping instead of 3d
        // grid1.mapper(std::bind(&raytrace::objects::cylinder::map, &cyl1, std::placeholders::_1));
        // cyl1.material(&grid1);

        cap.material(&plain_yellow);
        // 2d mapping instead of 3d
        polka_dots.mapper(std::bind(&raytrace::objects::cuboid::map, &cube0, std::placeholders::_1));
        cube0.material(&polka_dots);

        // plane is already 2d, no mapping needed
        // grid1.mapper(std::bind(&raytrace::objects::square::map, &floor, std::placeholders::_1));
        // floor.material(&grid1);
        grid1.mapper(std::bind(&raytrace::objects::plane::map, &ground, std::placeholders::_1));
        ground.material(&grid1);
        ground.set_surface_scale(1.0_p, 1.0_p/32);
        // ground.material(&plain_green);

        tri0.material(&plain_cyan);
        tri1.material(&plain_cyan);
        cone1.material(&plain_red);
        bounding_cone.material(&plain_red);
        cone2.material(&plain_red);
        // 2d mapping instead of 3d
        grey_checkers.mapper(std::bind(&raytrace::objects::torus::map, &torus1, std::placeholders::_1));
        torus1.material(&grey_checkers);

        // assign any rotation or translations

        // s2.rotation(iso::radians{0}, iso::radians{0}, iso::radians{iso::pi});
        // cube0.rotation(iso::degrees(45), iso::degrees(45), iso::degrees(45));
    }

    ~ExampleWorld() = default;

    raytrace::point& looking_from() override {
        return look_from;
    }

    raytrace::point& looking_at() override {
        return look_at;
    }

    std::string window_name() const override {
        return std::string("Raytracing Example");
    }

    std::string output_filename() const override {
        return std::string("world_example.tga");
    }

    raytrace::color background(raytrace::ray const&) const override {
        return colors::black;
    }

    void add_to(scene& scene) override {
        // add the objects to the scene.
        scene.add_object(&ground);
        // scene.add_object(&floor);
        scene.add_object(&cube0);
        scene.add_object(&cyl1);
        scene.add_object(&cap);
        // scene.add_object(&cone1);
        scene.add_object(&cone2);
        scene.add_object(&s2);
        scene.add_object(&ring0);
        scene.add_object(&ring1);
        scene.add_object(&tri0);
        scene.add_object(&tri1);
        scene.add_object(&torus1);

        // add lights to the scene
        // scene.add_light(&blue_light);
        // scene.add_light(&red_light);
        // scene.add_light(&green_light);
        scene.add_light(&sunlight);
        scene.add_light(&white_light);
    }

protected:
    raytrace::point look_from;
    raytrace::point look_at;
    vector tilt;
    // define some surfaces
    plain plain_green;
    plain plain_yellow;
    plain plain_blue;
    plain plain_red;
    plain plain_cyan;
    checkerboard grey_checkers;
    checkerboard ikea_checkers;
    dots polka_dots;
    turbsin bw_marble;
    grid grid1;
    // sphere w/ ring
    precision s2r;
    precision r0r1;
    precision r0r2;
    raytrace::point pos0;
    raytrace::objects::sphere s2;
    ring ring0;
    ring ring1;
    raytrace::point pos1;
    raytrace::point top;
    cylinder cyl1;
    ring cap;
    raytrace::point po2;
    cuboid cube0;
    raytrace::objects::plane ground;
    square floor;
    raytrace::point pos5;
    raytrace::point pos6;
    raytrace::point pos7;
    triangle tri0;
    triangle tri1;
    raytrace::point pos8;
    cone cone1;
    cuboid bounding_cone;
    overlap cone2;
    raytrace::point pos9;
    torus torus1;
    // create some lights
    speck blue_light;
    speck red_light;
    speck green_light;
    speck white_light;
    beam sunlight;
};

// declare a single instance and return the reference to it
world* get_world() {
    static ExampleWorld my_world;
    return &my_world;
}
