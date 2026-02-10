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
        : world{raytrace::point{40, 9, 37}, raytrace::point{0, 0, 15}, "Monochrome World 4", "world_monochrome_4.tga"}
        , light_subsamples{5}
        , light0{raytrace::point{-5, 0, 40}, 1, colors::white, 10, light_subsamples}
        , light1{raytrace::point{5, 0, 40}, 1, colors::white, 10, light_subsamples}
        , beam0{-R3::basis::Z, colors::white, 10E11}
        , s0{raytrace::point{0, 0, 5}, 5}
        , s1{raytrace::point{0, 0, 15}, 5}
        , s2{raytrace::point{0, 0, 25}, 5}
        , wall0{raytrace::point{0, 10, 0}, R3::roll(iso::radians{iso::pi / 2})}    // left
        , wall1{raytrace::point{0, -10, 0}, R3::roll(iso::radians{-iso::pi / 2})}  // right
        , wall2{raytrace::point{-10, 0, 0}, R3::pitch(iso::radians{iso::pi / 2})}  // back
        , wall3{}                                                                  // floor
        , wall4{raytrace::point{0, 0, 160}, R3::pitch(iso::radians{iso::pi})}      // ceiling
    {
        // assign surfaces and materials
        s0.material(&mediums::metals::silver);
        s1.material(&mediums::metals::silver);
        s2.material(&mediums::metals::silver);
        wall0.material(&mediums::metals::copper);
        wall1.material(&mediums::metals::copper);
        wall2.material(&mediums::metals::copper);
        wall3.material(&mediums::metals::copper);
        wall4.material(&mediums::metals::copper);
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
        scene.add_object(&s1);
        scene.add_object(&s2);
        scene.add_object(&wall0);
        scene.add_object(&wall1);
        scene.add_object(&wall2);
        scene.add_object(&wall3);
        // scene.add_object(&wall4);
    }

    raytrace::animation::anchors get_anchors() const override {
        raytrace::animation::anchors anchors;
        anchors.push_back(animation::Anchor{animation::Attributes{looking_from(), looking_at(), 55.0_deg},
                                            animation::Attributes{looking_from(), looking_at(), 55.0_deg}, animation::Mappers{},
                                            iso::seconds{1.0_p}});
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
    raytrace::objects::sphere s1;
    raytrace::objects::sphere s2;
    // backwalls
    raytrace::objects::plane wall0;
    raytrace::objects::plane wall1;
    raytrace::objects::plane wall2;
    raytrace::objects::plane wall3;
    raytrace::objects::plane wall4;
};

// declare a single instance and return the reference to it
world* get_world() {
    static MonochromeWorld my_world;
    return &my_world;
}
