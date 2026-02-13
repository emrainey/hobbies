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
        : world{raytrace::point{15, -20, 0}, raytrace::point{20, 20, 15}, "Monochrome World 3",
                "world_monochrome_3.tga"}
        , light_subsamples{5}
        , light0{raytrace::point{-5, 0, 40}, 1, colors::white, 10, light_subsamples}
        , light1{raytrace::point{5, 0, 40}, 1, colors::white, 10, light_subsamples}
        , beam0{raytrace::vector{10, 10, -100}, colors::white, 10E11}
        , s0{raytrace::point{5, 20, 15}, 6}
        , t0{raytrace::point{-2, 20, 17}, 3, 1}              // , o0{}
                                                             // coordinates in overlap space
        , c0{raytrace::point{0, 0, 0}, 100, 100, 100}        // coordinates in overlap space
        , c1{raytrace::point{-15, -100, 35}, 10, 2.5_p, 10}  // coordinates in world space
        , c2{raytrace::point{15, 20, 35}, 10, 2.5_p, 10}
        , o1{c0, c1, raytrace::objects::overlap::type::subtractive}
        , p0{raytrace::point{20, 20, 15}, 5} {
        // assign surfaces and materials
        s0.material(&mediums::metals::stainless);
        t0.material(&mediums::metals::stainless);
        c0.material(&mediums::metals::stainless);
        c1.material(&mediums::metals::stainless);
        o1.material(&mediums::metals::stainless);
        // overlaps compute a centroid for their position which could be off-origin
        // assign it's location here which will then "move" the other subobjects
        o1.position(raytrace::point{0, 120, 0});
        c2.material(&mediums::metals::stainless);
        p0.material(&mediums::metals::stainless);
        p0.rotation(iso::degrees{90}, iso::degrees{45}, iso::degrees{0});
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
        scene.add_light(&beam0);
        scene.add_object(&s0);
        scene.add_object(&t0);
        // scene.add_object(&o0);
        // scene.add_object(&c0);
        // scene.add_object(&c1);
        scene.add_object(&o1);
        scene.add_object(&c2);
        scene.add_object(&p0);
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
    raytrace::lights::beam beam0;
    // objects
    raytrace::objects::sphere s0;
    raytrace::objects::torus t0;
    // raytrace::objects::overlap o0; // s0 + t0
    raytrace::objects::cuboid c0;
    raytrace::objects::cuboid c1;   // sunk in
    raytrace::objects::cuboid c2;   // sticks out
    raytrace::objects::overlap o1;  // c0 - c1
    raytrace::objects::pyramid p0;
};

// declare a single instance and return the reference to it
world* get_world() {
    static MonochromeWorld my_world;
    return &my_world;
}
