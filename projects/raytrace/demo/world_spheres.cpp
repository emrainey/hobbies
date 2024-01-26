/**
 * @file
 * @author Erik Rainey (erik.rainey@gmail.com)
 * @brief Renders a raytraced image of a bunch of spheres
 * @date 2021-01-02
 * @copyright Copyright (c) 2021
 */

#include <raytrace/raytrace.hpp>

#include "world.hpp"

using namespace raytrace;

void subspheres(std::vector<raytrace::objects::sphere*>& spheres, const raytrace::point& center, precision R,
                size_t depth) {
    if (depth > 0) {
        // adding 28 spheres of
        precision radius = R / 6.0_p;
        for (precision z = -R; z <= R; z += R) {
            for (precision y = -R; y <= R; y += R) {
                for (precision x = -R; x <= R; x += R) {
                    // if all are zero, continue
                    if (iso::equivalent(x, 0.0_p) and iso::equivalent(y, 0.0_p) and iso::equivalent(z, 0.0_p)) {
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
        , spheres{}
        , sunlight{raytrace::vector{-2, 2, -1}, colors::white, 1E11}
        , specks{} {
        raytrace::point center = look_at;
        spheres.push_back(new raytrace::objects::sphere(center, 6));
        subspheres(spheres, center, 12.0_p, 2);
        for (auto& s : spheres) {
            s->material(&mediums::metals::stainless);
        }
        specks.push_back(new lights::speck(raytrace::point{80, 120, 80}, colors::white, 1E11));
        specks.push_back(new lights::speck(raytrace::point{80, 40, 80}, colors::white, 1E11));
        specks.push_back(new lights::speck(raytrace::point{40, 80, 80}, colors::white, 1E11));
        specks.push_back(new lights::speck(raytrace::point{120, 80, 80}, colors::white, 1E11));
    }

    ~SpheresWorld() {
        for (auto& s : spheres) {
            delete s;
        }
        for (auto& s : specks) {
            delete s;
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

    raytrace::color background(const raytrace::ray&) const override {
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
    }

protected:
    raytrace::point look_from;
    raytrace::point look_at;
    std::vector<raytrace::objects::sphere*> spheres;
    lights::beam sunlight;
    std::vector<raytrace::lights::speck*> specks;
};

// declare a single instance and return the reference to it
world* get_world() {
    static SpheresWorld my_world;
    return &my_world;
}
