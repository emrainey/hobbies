#include <raytrace/raytrace.hpp>

#include "world.hpp"

using namespace raytrace;
using namespace raytrace::objects;
using namespace raytrace::lights;
using namespace raytrace::colors;
using namespace raytrace::operators;

// Good results at values of depth=8, subsamples=16, 1024x768, fov=23
class ArtWorld : public world {
public:
    ArtWorld()
        : look_from{0, 25, 10}
        , look_at{0, 0, 10}
        , sun_rays{raytrace::vector{-20, 0, -21}, colors::white, lights::intensities::dim}
        , floor{R3::origin, R3::basis::Z, 100.0, 100.0}
        , pyramid{look_at, 0}
        , orb{raytrace::point{0, 0, 12}, 2.0}
        , halo{raytrace::point{0, 0, 14}, 2.25, 0.125} {
        pyramid.material(&mediums::metals::stainless);
        orb.material(&mediums::metals::stainless);
        floor.material(&mediums::metals::stainless);
        halo.material(&mediums::metals::copper);
    }

    ~ArtWorld() = default;

    raytrace::point& looking_from() override {
        return look_from;
    }

    raytrace::point& looking_at() override {
        return look_at;
    }

    std::string window_name() const override {
        return std::string("Art1 World");
    }

    std::string output_filename() const override {
        return std::string("world_art1.tga");
    }

    raytrace::color background(raytrace::ray const& world_ray) const override {
        // this creates a gradient from top to bottom
        iso::radians sky_angle = angle(R3::basis::Z, world_ray.direction());
        precision scalar = sky_angle.value / iso::pi;
        return interpolate(colors::dark_slate_blue, colors::light_blue, scalar);
        // return colors::black;
    }

    void add_to(scene& scene) override {
        // scene.add_light(&inner_light);
        scene.add_light(&sun_rays);
        scene.add_object(&floor);
        scene.add_object(&pyramid);
        scene.add_object(&orb);
        scene.add_object(&halo);
        scene.add_media(&mediums::earth_atmosphere);
    }

protected:
    raytrace::point look_from;
    raytrace::point look_at;
    lights::beam sun_rays;
    raytrace::point center;
    raytrace::objects::square floor;
    raytrace::objects::pyramid pyramid;
    raytrace::objects::sphere orb;
    // raytrace::objects::cone    orb_cut;
    // raytrace::objects::cuboid  column;
    // raytrace::objects::pyramid column_cap;
    raytrace::objects::torus halo;
};

// declare a single instance and return the reference to it
world* get_world() {
    static ArtWorld my_world;
    return &my_world;
}
