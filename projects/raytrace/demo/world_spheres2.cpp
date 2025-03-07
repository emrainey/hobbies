///
/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Renders a raytraced image of a bunch of spheres in a halo around a central sphere.
/// @date 2021-01-02
/// @copyright Copyright (c) 2021
///

#include <raytrace/raytrace.hpp>

#include "world.hpp"

using namespace raytrace;
using namespace iso::literals;

using mat = const mediums::metal;

raytrace::mediums::metal const* my_metals[]
    = {&raytrace::mediums::metals::aluminum, &raytrace::mediums::metals::brass,     &raytrace::mediums::metals::bronze,
       &raytrace::mediums::metals::chrome,   &raytrace::mediums::metals::copper,    &raytrace::mediums::metals::gold,
       &raytrace::mediums::metals::silver,   &raytrace::mediums::metals::stainless, &raytrace::mediums::metals::steel,
       &raytrace::mediums::metals::tin};

void subspheres(std::vector<raytrace::objects::sphere*>& spheres, raytrace::point const& center, precision R,
                precision sR, size_t limit) {
    for (size_t s = 0; s < limit; s++) {
        R3::point pnt = raytrace::mapping::golden_ratio_mapper(s, limit);
        R3::vector dir = pnt - R3::origin;
        pnt = pnt + (dir * R);  // move the point out from the origin by R
        spheres.push_back(new raytrace::objects::sphere(pnt, sR));
    }
}

void materials(std::vector<mat*>& mats, size_t limits) {
    for (size_t m = 0; m < limits; m++) {
        int r = rand() % dimof(my_metals);
        mats.push_back(my_metals[r]);
    }
}

class Spheres2World : public world {
public:
    Spheres2World()
        : world{}
        , look_from{20, 0, 20}
        , look_at{0, 0, 0}
        , number_of_spheres{128}
        , spheres{}
        , mats{}
        , sunlight{raytrace::vector{-2, 2, -1}, colors::white, lights::intensities::bright}
        , specks{} {
        raytrace::point center = look_at;
        spheres.push_back(new raytrace::objects::sphere(center, 4.5));
        subspheres(spheres, center, 6, 0.4, number_of_spheres);
        materials(mats, spheres.size());
        for (size_t s = 0; s < number_of_spheres; s++) {
            spheres[s]->material(mats[s]);
        }
        // specks.push_back(new lights::speck(raytrace::point{80, 120, 80}, colors::white,
        // lights::intensities::bright)); specks.push_back(new lights::speck(raytrace::point{80, 40, 80}, colors::white,
        // lights::intensities::bright)); specks.push_back(new lights::speck(raytrace::point{40, 80, 80}, colors::white,
        // lights::intensities::bright)); specks.push_back(new lights::speck(raytrace::point{120, 80, 80},
        // colors::white, 1E11));
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
        return std::string("world_spheres2.tga");
    }

    raytrace::color background(raytrace::ray const& world_ray) const override {
        iso::radians A = angle(R3::basis::Z, world_ray.direction());
        precision B = A.value / iso::pi;
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
    size_t number_of_spheres;
    std::vector<raytrace::objects::sphere*> spheres;
    std::vector<mat*> mats;
    lights::beam sunlight;
    std::vector<raytrace::lights::speck*> specks;
};

// declare a single instance and return the reference to it
world* get_world() {
    static Spheres2World my_world;
    return &my_world;
}
