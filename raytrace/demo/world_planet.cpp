#include <raytrace/raytrace.hpp>
#include "world.hpp"

using namespace raytrace;
using namespace raytrace::colors;
using namespace raytrace::operators;


class PlanetWorld : public world {
public:
    PlanetWorld()
        : look_from(50, 0, 34)
        , look_at(8, 8, 10)
        //, reflection_depth(8)
        //, subsamples(16)
        //, image_height(768)
        //, image_width(1024)
        //, field_of_view(23)
        , steel(colors::white, smoothness::polished, roughness::loose)
        , beam_of_light(raytrace::vector{-20, 0, -21}, colors::white, 1E3)
        , inner_light(raytrace::point(0, 0, 80), colors::white, 1E11)
        , center(0, 0, 10)
        , ringA(center, R3::basis::Z, 10.0, 11.8)
        , ringB(center, R3::basis::Z, 12.0, 12.2)
        , ringC(center, R3::basis::Z, 12.4, 13.2)
        , ringD(center, R3::basis::Z, 14.0, 14.2)
        , ringE(center, R3::basis::Z, 14.5, 14.7)
        , ringF(center, R3::basis::Z, 15.0, 16.7)
        , planet(center, 7.5)
        {
        planet.material(&steel);
        ringA.material(&steel);
        ringB.material(&steel);
        ringC.material(&steel);
        ringD.material(&steel);
        ringE.material(&steel);
        ringF.material(&steel);
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
        return std::string("world_planet.ppm");
    }

    raytrace::color background(const raytrace::ray& world_ray) const override {
        return colors::black;
    }

    void add_to(scene& scene) override {
        scene.add_light(&inner_light);
        scene.add_object(&planet);
        scene.add_object(&ringA);
        scene.add_object(&ringB);
        scene.add_object(&ringC);
        scene.add_object(&ringD);
        scene.add_object(&ringE);
        scene.add_object(&ringF);
    }

protected:
    raytrace::point look_from;
    raytrace::point look_at;
    mediums::metal steel;
    beam beam_of_light;
    speck inner_light;
    R3::point center;
    raytrace::ring ringA;
    raytrace::ring ringB;
    raytrace::ring ringC;
    raytrace::ring ringD;
    raytrace::ring ringE;
    raytrace::ring ringF;
    raytrace::sphere planet;
};

// declare a single instance and return the reference to it
world* get_world() {
    static PlanetWorld my_world;
    return &my_world;
}
