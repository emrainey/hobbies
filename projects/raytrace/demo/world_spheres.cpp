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
                    if (basal::equivalent(x, 0.0_p) and basal::equivalent(y, 0.0_p) and basal::equivalent(z, 0.0_p)) {
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
        : world{}  //, look_from(-50, 50, 20)
                   //, look_at(0, 0, 0)
        , look_from{-10.0_p, 6.66_p, 20.0_p}
        , look_at{3.0_p, 0.0_p, 6.0_p}
        , custom_grey{0.75_p, 0.75_p, 0.75_p}
        , custom_metal{custom_grey, raytrace::mediums::smoothness::mirror, raytrace::mediums::roughness::medium}
        , spheres{}
        , specks{}
        , bulbs{} {
        raytrace::point center = R3::origin;
        spheres.push_back(new raytrace::objects::sphere(center, 6));
        subspheres(spheres, center, 12.0_p, 2);
        for (auto& s : spheres) {
            // s->material(&custom_metal);
            s->material(&mediums::metals::stainless);
        }
        specks.push_back(new lights::speck(raytrace::point{+6, +6, 9}, colors::white, lights::intensities::blinding));
        specks.push_back(new lights::speck(raytrace::point{-6, +6, 9}, colors::white, lights::intensities::blinding));
        specks.push_back(new lights::speck(raytrace::point{-6, -6, 9}, colors::white, lights::intensities::blinding));
        specks.push_back(new lights::speck(raytrace::point{+6, -6, 9}, colors::white, lights::intensities::blinding));
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

    raytrace::point& looking_from() override {
        return look_from;
    }

    raytrace::point& looking_at() override {
        return look_at;
    }

    std::string window_name() const override {
        return std::string("Spheres");
    }

    std::string output_filename() const override {
        return std::string("world_spheres.tga");
    }

    raytrace::color background(raytrace::ray const&) const override {
        return colors::black;
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
        anchors.push_back(animation::Anchor{animation::Attributes{look_from, look_at, 55.0_deg},
                                            animation::Attributes{look_from, look_at, 55.0_deg}, animation::Mappers{},
                                            iso::seconds{1.0_p}});
        return anchors;
    }

protected:
    raytrace::point look_from;
    raytrace::point look_at;
    raytrace::color custom_grey;
    raytrace::mediums::metal custom_metal;
    std::vector<raytrace::objects::sphere*> spheres;
    std::vector<raytrace::lights::speck*> specks;
    std::vector<raytrace::lights::bulb*> bulbs;
};

// declare a single instance and return the reference to it
world* get_world() {
    static SpheresWorld my_world;
    return &my_world;
}
