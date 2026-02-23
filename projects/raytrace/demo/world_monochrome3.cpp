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
        : world{raytrace::point{0, -20, 0}, raytrace::point{0, 0, 0}, "Monochrome World 3",
                "world_monochrome_3.tga"}
        , light_subsamples{5}
        , light0{raytrace::point{-7, 0, 0}, 1, colors::white, 30 * lights::intensities::bright, light_subsamples}
        , light1{raytrace::point{7, 0, 0}, 1, colors::white, 30 * lights::intensities::bright, light_subsamples}
        , s0{raytrace::point{0, 0, 0}, 5.0_p}
        , s1{raytrace::point{7, 7, 0}, 2.0_p}
        , s2{raytrace::point{-7, 7, 0}, 2.0_p}
        , s3{raytrace::point{-7, -7, 0}, 2.0_p}
        , s4{raytrace::point{7, -7, 0}, 2.0_p}
        , cone0{raytrace::point{0, 0, 0}, 0.5_p, 0.5_p}
        , torus0{raytrace::point{0, 0, 5}, 3.0_p, 0.5_p}
        , torus1{raytrace::point{0, 0, -5}, 3.0_p, 0.5_p}
        , cube0{raytrace::point{0, -27, 0}, 5.0_p, 5.0_p, 5.0_p} {

        // assign surfaces and materials
        s0.material(&mediums::metals::stainless);
        s1.material(&mediums::metals::copper);
        s2.material(&mediums::metals::copper);
        s3.material(&mediums::metals::copper);
        s4.material(&mediums::metals::copper);
        cone0.material(&mediums::metals::bronze);
        torus0.material(&mediums::metals::stainless);
        torus1.material(&mediums::metals::stainless);
        cube0.material(&mediums::metals::gold);
        cube0.rotation(iso::radians{iso::pi/4}, iso::radians{0}, iso::radians{iso::pi / 4});
    }

    ~MonochromeWorld() = default;

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
        scene.add_object(&s0);
        scene.add_object(&s1);
        scene.add_object(&s2);
        scene.add_object(&s3);
        scene.add_object(&s4);
        scene.add_object(&cone0);
        scene.add_object(&torus0);
        scene.add_object(&torus1);
        scene.add_object(&cube0);
    }

    raytrace::animation::anchors get_anchors() const override {
        raytrace::animation::anchors anchors;
        anchors.push_back(animation::Anchor{animation::Attributes{looking_from(), looking_at(), iso::degrees{55.0_p}},
                                            animation::Attributes{looking_from(), looking_at(), iso::degrees{55.0_p}},
                                            animation::Mappers{}, iso::seconds{1.0_p}});
        return anchors;
    }

protected:
    size_t light_subsamples;
    // lights
    raytrace::lights::bulb light0;
    raytrace::lights::bulb light1;
    // objects
    raytrace::objects::sphere s0;
    raytrace::objects::sphere s1;
    raytrace::objects::sphere s2;
    raytrace::objects::sphere s3;
    raytrace::objects::sphere s4;
    raytrace::objects::ellipticalcone cone0;
    raytrace::objects::torus torus0;
    raytrace::objects::torus torus1;
    raytrace::objects::cuboid cube0;

};

// declare a single instance and return the reference to it
world* get_world() {
    static MonochromeWorld my_world;
    return &my_world;
}
