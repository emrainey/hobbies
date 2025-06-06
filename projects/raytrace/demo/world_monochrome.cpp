///
/// @file
/// The Steel Object Scene from year ago with Vivid 3.
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

class MonochromeWorld : public world {
public:
    MonochromeWorld()
        : light_subsamples{20}
        , look_from{5, -20, 20}
        , look_at{0, 0, 0}
        , plain_white{colors::white, mediums::ambient::none, colors::white, mediums::smoothness::none, roughness::tight}
        , checkerboard_grid{0.1_p,         colors::blue, colors::yellow, colors::red,  colors::magenta,
                            colors::green, colors::cyan, colors::black,  colors::white}
        , floor{200.0_p}
        , light0{raytrace::point{-5, 0, 10}, 1, colors::white, 10, light_subsamples}
        , light1{raytrace::point{-4, 0, 10}, 1, colors::white, 10, light_subsamples}
        , light2{raytrace::point{-3, 0, 10}, 1, colors::white, 10, light_subsamples}
        , light3{raytrace::point{-2, 0, 10}, 1, colors::white, 10, light_subsamples}
        , light4{raytrace::point{-1, 0, 10}, 1, colors::white, 10, light_subsamples}
        , light5{raytrace::point{0, 0, 10}, 1, colors::white, 10, light_subsamples}
        , light6{raytrace::point{1, 0, 10}, 1, colors::white, 10, light_subsamples}
        , light7{raytrace::point{2, 0, 10}, 1, colors::white, 10, light_subsamples}
        , light8{raytrace::point{3, 0, 10}, 1, colors::white, 10, light_subsamples}
        , light9{raytrace::point{4, 0, 10}, 1, colors::white, 10, light_subsamples}
        , light10{raytrace::point{5, 0, 10}, 1, colors::white, 10, light_subsamples}
        , s1{raytrace::point{4, 2, 1}, 1}
        , s2{raytrace::point{-4, -2, 2}, 2}
        , s3{raytrace::point{1, -5, 3}, 3}
        , c1{raytrace::point{6, 3, 0}, 1, 4}  // cone
        //, infinite_cylinder{raytrace::point{-2, 3, 2}, 1, 1} // cylinder
        //, cylinder_box{raytrace::point{-2, 3, 2}, 1, 1, 2}
        //, cyl{infinite_cylinder, cylinder_box, overlap::type::inclusive}
        , cyl{raytrace::point{-2, 3, 2}, 2, 1}  // cylinder
        , cap{raytrace::point{-2, 3, 4}, R3::identity, 0, 1}
        , t0{raytrace::point{3, 7, 0.5_p}, 1.4_p, 0.5_p}
        , cb0{raytrace::point{7, -2, 1}, 1, 1, 1} {
        // assign surfaces and materials
        floor.material(&plain_white);
        checkerboard_grid.mapper(std::bind(&objects::square::map, &floor, std::placeholders::_1));
        // floor.material(&checkerboard_grid);
        s1.material(&mediums::metals::stainless);
        s2.material(&mediums::metals::stainless);
        s3.material(&mediums::metals::stainless);
        c1.material(&mediums::metals::stainless);
        cyl.material(&mediums::metals::stainless);
        cap.material(&mediums::metals::stainless);
        t0.material(&mediums::metals::stainless);
        cb0.material(&mediums::metals::stainless);
        cb0.rotation(iso::degrees{0}, iso::degrees{0}, iso::degrees{15});
    }

    raytrace::point& looking_from() override {
        return look_from;
    }

    raytrace::point& looking_at() override {
        return look_at;
    }

    std::string window_name() const override {
        return std::string("Monochrome World");
    }

    std::string output_filename() const override {
        return std::string("world_monochrome.tga");
    }

    raytrace::color background(raytrace::ray const& world_ray) const override {
        // this creates a gradient from top to bottom
        iso::radians A = angle(R3::basis::Z, world_ray.direction());
        precision B = A.value / iso::pi;
        return color(0.8_p * B, 0.8_p * B, 0.8_p * B);
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
        scene.add_object(&floor);
        scene.add_object(&s1);
        scene.add_object(&s2);
        scene.add_object(&s3);
        scene.add_object(&c1);
        scene.add_object(&cyl);
        scene.add_object(&cap);
        scene.add_object(&t0);
        scene.add_object(&cb0);
    }

    raytrace::animation::anchors get_anchors() const override {
        raytrace::animation::anchors anchors;
        anchors.push_back(animation::Anchor{animation::Attributes{look_from, look_at, 55.0_deg},
                                            animation::Attributes{look_from, look_at, 55.0_deg}, animation::Mappers{},
                                            iso::seconds{1.0_p}});
        return anchors;
    }

protected:
    size_t light_subsamples;
    raytrace::point look_from;
    raytrace::point look_at;
    mediums::plain plain_white;
    mediums::checkerboard checkerboard_grid;
    raytrace::objects::square floor;
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
    raytrace::objects::sphere s1;
    // ellipsoid s1;
    raytrace::objects::sphere s2;
    raytrace::objects::sphere s3;
    cone c1;
    cylinder cyl;
    // ellipticalcylinder infinite_cylinder;
    // cuboid   cylinder_box;
    // overlap  cyl;
    ring cap;
    torus t0;
    cuboid cb0;
};

// declare a single instance and return the reference to it
world* get_world() {
    static MonochromeWorld my_world;
    return &my_world;
}
