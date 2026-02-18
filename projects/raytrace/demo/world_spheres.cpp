///
/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Renders a raytraced image of a bunch of spheres
/// @date 2021-01-02
/// @copyright Copyright (c) 2021
///

#include <raytrace/raytrace.hpp>

#include "world.hpp"

using namespace raytrace;
using namespace iso::literals;

void subspheres(std::vector<raytrace::objects::sphere*>& spheres, raytrace::point const& center, precision R,
                size_t depth) {
    if (depth > 0) {
        precision radius = R / 6.0_p;
        for (precision z = -R; z <= R; z += R) {
            for (precision y = -R; y <= R; y += R) {
                for (precision x = -R; x <= R; x += R) {
                    // if all are zero, continue
                    if (basal::nearly_zero(x) and basal::nearly_zero(y) and basal::nearly_zero(z)) {
                        continue;
                    }
                    spheres.push_back(new raytrace::objects::sphere(center + R3::vector{{x, y, z}}, radius));
                    subspheres(spheres, spheres.back()->position(), R / 3.0_p, depth - 1);
                }
            }
        }
    }
}
class SpheresWorld : public world {
public:
    SpheresWorld()
        : world{raytrace::point{-10.0_p, 6.66_p, 20.0_p}, raytrace::point{3.0_p, 0.0_p, 6.0_p}, "Spheres World",
                "world_spheres.tga"}
        , custom_grey{0.975_p, 0.97_p, 0.97_p}
        , custom_metal{colors::white, raytrace::mediums::smoothness::barely, raytrace::mediums::roughness::loose, 0.25_p}
        , custom_plain{colors::white, 1.0_p / 8.0_p, colors::green, raytrace::mediums::smoothness::barely, raytrace::mediums::roughness::very_loose}
        , spheres{}
        , specks{}
        , bulbs{} {
        raytrace::point center = R3::origin;
        spheres.push_back(new raytrace::objects::sphere(center, 6));
        subspheres(spheres, center, 12.0_p, 3);
        for (auto& s : spheres) {
            // s->material(&custom_metal);
            // s->material(&custom_plain);
            s->material(&mediums::metals::stainless);
        }
        precision intensity = 60.0_p;
        specks.push_back(new lights::speck(raytrace::point{+6.0_p, +6.0_p, 9.0_p}, colors::white, intensity));
        specks.push_back(new lights::speck(raytrace::point{-6.0_p, +6.0_p, 9.0_p}, colors::white, intensity));
        specks.push_back(new lights::speck(raytrace::point{-6.0_p, -6.0_p, 9.0_p}, colors::white, intensity));
        specks.push_back(new lights::speck(raytrace::point{+6.0_p, -6.0_p, 9.0_p}, colors::white, intensity));
        // bulbs.push_back(
        //     new lights::bulb(raytrace::point{+6, +6, 9}, 1.0_p, colors::white, lights::intensities::blinding, 16));
        // bulbs.push_back(
        //     new lights::bulb(raytrace::point{-6, +6, 9}, 1.0_p, colors::white, lights::intensities::blinding, 16));
        // bulbs.push_back(
        //     new lights::bulb(raytrace::point{-6, -6, 9}, 1.0_p, colors::white, lights::intensities::blinding, 16));
        // bulbs.push_back(
        //     new lights::bulb(raytrace::point{+6, -6, 9}, 1.0_p, colors::white, lights::intensities::blinding, 16));
    }

    ~SpheresWorld() {
        for (auto& s : spheres) {
            delete s;
        }
        for (auto& s : specks) {
            delete s;
        }
        for (auto& b : bulbs) {
            delete b;
        }
    }

    void add_to(scene& scene) override {
        // add the objects to the scene.
        for (auto& s : spheres) {
            scene.add_object(s);
        }
        for (auto& s : specks) {
            scene.add_light(s);
        }
        for (auto& b : bulbs) {
            scene.add_light(b);
        }
    }

    raytrace::animation::anchors get_anchors() const override {
        raytrace::animation::anchors anchors;
        anchors.push_back(animation::Anchor{animation::Attributes{looking_from(), looking_at(), 55.0_deg},
                                            animation::Attributes{looking_from(), looking_at(), 55.0_deg},
                                            animation::Mappers{}, iso::seconds{1.0_p}});
        return anchors;
    }

protected:
    raytrace::color custom_grey;
    raytrace::mediums::metal custom_metal;
    raytrace::mediums::plain custom_plain;
    std::vector<raytrace::objects::sphere*> spheres;
    std::vector<raytrace::lights::speck*> specks;
    std::vector<raytrace::lights::bulb*> bulbs;
};

// declare a single instance and return the reference to it
world* get_world() {
    static SpheresWorld my_world;
    return &my_world;
}
