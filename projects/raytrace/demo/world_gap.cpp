///
/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Renders a scene with a gapped wall with sphere emerging.
/// @date 2024-08-10
/// @copyright Copyright (c) 2024
///

#include <raytrace/raytrace.hpp>

#include "world.hpp"

using namespace raytrace;
using namespace raytrace::objects;
using namespace raytrace::lights;
using namespace iso::literals;

class GapWallWorld : public world {
public:
    GapWallWorld()
        : world{raytrace::point{-30, -50, 100}, raytrace::point{0, 0, 0}, "Raytracing Wall Gap w/ Sphere", "world_gap.tga"}
        , wall1{R3::origin, R3::roll(iso::radians{-iso::pi / 2}), 20.0_p}
        , subgap{R3::origin, R3::pitch(iso::radians{iso::pi / 2}), 30.0_p}
        , gap{wall1, subgap, raytrace::objects::overlap::type::subtractive}
        , sphere{R3::origin, 15}
        , sunlight{R3::vector{0, 100, -100}, raytrace::colors::white, lights::intensities::blinding} {
        wall1.material(&raytrace::mediums::metals::copper);
        // wall1.print("Wall1");
        subgap.material(&raytrace::mediums::metals::copper);
        // wall1.print("Gap in Wall1");
        gap.material(&raytrace::mediums::metals::copper);
        sphere.material(&raytrace::mediums::metals::silver);
    }

    ~GapWallWorld() = default;

    raytrace::color background(raytrace::ray const& world_ray) const override {
        iso::radians A = angle(R3::basis::Z, world_ray.direction());
        precision B = A.value / iso::pi;
        return color(0.8_p * B, 0.8_p * B, 0.8_p * B);
    }

    void add_to(scene& scene) override {
        // add the objects to the scene.
        // scene.add_object(&wall1);
        // scene.add_object(&subgap);
        scene.add_object(&gap);
        scene.add_object(&sphere);
        scene.add_light(&sunlight);
    }

    raytrace::animation::anchors get_anchors() const override {
        raytrace::animation::anchors anchors;
        anchors.push_back(animation::Anchor{animation::Attributes{looking_from(), looking_at(), 55.0_deg},
                                            animation::Attributes{looking_from(), looking_at(), 55.0_deg}, animation::Mappers{},
                                            iso::seconds{1.0_p}});
        return anchors;
    }

protected:
    raytrace::objects::wall wall1;
    raytrace::objects::wall subgap;
    raytrace::objects::overlap gap;
    raytrace::objects::sphere sphere;
    raytrace::lights::beam sunlight;
};

// declare a single instance and return the reference to it
world* get_world() {
    static GapWallWorld my_world;
    return &my_world;
}
