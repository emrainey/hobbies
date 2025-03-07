#include <raytrace/raytrace.hpp>
#include <raytrace/mediums/starfield.hpp>

#include "world.hpp"

using namespace raytrace;
using namespace raytrace::objects;
using namespace raytrace::lights;
using namespace raytrace::colors;
using namespace raytrace::operators;
using namespace iso::literals;

// Good results at values of depth=8, subsamples=16, 1024x768, fov=23
class PlanetWorld : public world {
public:
    PlanetWorld()
        : look_from{0, 50, 10},
          look_at{8, 0, 0},
          sun_rays{raytrace::vector{-20, 0, -21}, colors::white, lights::intensities::bright / 2},
          inner_light{raytrace::point{0, 0, 80}, colors::white, lights::intensities::blinding},
          center{0, 0, 0},
          ringA{center, R3::basis::Z, 10.0, 11.8},
          ringB{center, R3::basis::Z, 12.0, 12.2},
          ringC{center, R3::basis::Z, 12.4, 13.2},
          ringD{center, R3::basis::Z, 14.0, 14.2},
          ringE{center, R3::basis::Z, 14.5, 14.7},
          ringF{center, R3::basis::Z, 15.0, 16.7},
          planet{center, 1.0},
          starfield{1024} {
        planet.material(&mediums::metals::stainless);
        ringA.material(&mediums::metals::stainless);
        ringB.material(&mediums::metals::stainless);
        ringC.material(&mediums::metals::stainless);
        ringD.material(&mediums::metals::stainless);
        ringE.material(&mediums::metals::stainless);
        ringF.material(&mediums::metals::stainless);
        planet.scale(7.5_p, 7.5_p, 7.5_p);  // use scaling instead of a larger radius
    }

    ~PlanetWorld() = default;

    raytrace::point& looking_from() override {
        return look_from;
    }

    raytrace::point& looking_at() override {
        return look_at;
    }

    std::string window_name() const override {
        return std::string("Planet World");
    }

    std::string output_filename() const override {
        return std::string("world_planet.tga");
    }

    raytrace::color background(raytrace::ray const& world_ray) const override {
        // starfield background
        return starfield(world_ray.direction());
    }

    void add_to(scene& scene) override {
        // scene.add_light(&inner_light);
        scene.add_light(&sun_rays);
        scene.add_object(&planet);
        scene.add_object(&ringA);
        scene.add_object(&ringB);
        scene.add_object(&ringC);
        scene.add_object(&ringD);
        scene.add_object(&ringE);
        scene.add_object(&ringF);
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
    lights::beam sun_rays;
    lights::speck inner_light;
    raytrace::point center;
    objects::ring ringA;
    objects::ring ringB;
    objects::ring ringC;
    objects::ring ringD;
    objects::ring ringE;
    objects::ring ringF;
    objects::sphere planet;
    mediums::StarField starfield;
};

// declare a single instance and return the reference to it
world* get_world() {
    static PlanetWorld my_world;
    return &my_world;
}
