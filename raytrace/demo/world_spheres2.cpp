/**
 * @file
 * @author Erik Rainey (erik.rainey@gmail.com)
 * @brief Renders a raytraced image of a bunch of spheres
 * @date 2021-01-02
 * @copyright Copyright (c) 2021
 */

#include <raytrace/raytrace.hpp>
#include <basal/options.hpp>
#include "world.hpp"

using namespace raytrace;

using mat = const metal;

const raytrace::mediums::metal* my_metals[] = {
    &raytrace::metals::aluminum, &raytrace::metals::brass, &raytrace::metals::bronze, &raytrace::metals::chrome, &raytrace::metals::copper,
    &raytrace::metals::gold, &raytrace::metals::silver, &raytrace::metals::stainless, &raytrace::metals::steel, &raytrace::metals::tin
};

void subspheres(std::vector<raytrace::sphere *>& spheres, const raytrace::point& center, double R, double sR, size_t limit) {
    for (size_t s = 0; s < limit; s++) {
        R3::point pnt = mapping::golden_ratio_mapper(s, limit);
        R3::vector dir = pnt - R3::origin;
        pnt = pnt + (dir * R); // move the point out from the origin by R
        spheres.push_back(new raytrace::sphere(pnt, sR));
    }
}

void materials(std::vector<mat *>& mats, size_t limits) {
    for (size_t m = 0; m < limits; m++) {
        // various metals?
        int r = rand() % dimof(my_metals);
        mats.push_back(my_metals[r]);
        // plain (plastic?)
        //mats.push_back(new plain(colors::white, ambient::none, color::random(), smoothness::barely, roughness::tight));
    }
}

class Spheres2World : public world {
public:
    Spheres2World()
        : world()
        , look_from(20, 0, 20)
        , look_at(0, 0, 0)
        , number_of_spheres(64)
        , spheres()
        , mats()
        , sunlight(raytrace::vector{-2, 2, -1}, colors::white, 1E11)
        , specks()
    {
        raytrace::point center = look_at;
        spheres.push_back(new raytrace::sphere(center, 4.5));
        subspheres(spheres, center, 6, 1.2, number_of_spheres);
        materials(mats, spheres.size());
        for (size_t s = 0; s < number_of_spheres; s++) {
            spheres[s]->material(mats[s]);
        }
        //specks.push_back(new speck(raytrace::point(80, 120, 80), colors::white, 1E11));
        //specks.push_back(new speck(raytrace::point(80, 40, 80), colors::white, 1E11));
        //specks.push_back(new speck(raytrace::point(40, 80, 80), colors::white, 1E11));
        //specks.push_back(new speck(raytrace::point(120, 80, 80), colors::white, 1E11));
    }

    ~Spheres2World() {
        for (auto& s : spheres) {
            delete s;
        }
        for (auto& s : mats) {
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
        return std::string("Spheres 2");
    }

    std::string output_filename() const override {
        return std::string("world_spheres2.ppm");
    }

    raytrace::color background(const raytrace::ray& world_ray) const override {
        iso::radians A = angle(R3::basis::Z, world_ray.direction());
        element_type B = A.value / iso::pi;
        return color(0.8 * B, 0.8 * B, 0.8 * B);
    }

    void add_to(scene& scene) override {
        // add the objects to the scene.
        for (auto& s : spheres) {
            scene.add_object(s);
        }
        for (auto& s : specks) {
            scene.add_light(s);
        }
        scene.add_light(&sunlight);
    }

protected:
    raytrace::point look_from;
    raytrace::point look_at;
    size_t number_of_spheres;
    std::vector<raytrace::sphere *> spheres;
    std::vector<mat *> mats;
    beam sunlight;
    std::vector<raytrace::speck *>specks;
};

// declare a single instance and return the reference to it
world* get_world() {
    static Spheres2World my_world;
    return &my_world;
}
