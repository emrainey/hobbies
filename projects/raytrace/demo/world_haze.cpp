#include <raytrace/raytrace.hpp>

#include "world.hpp"

using namespace raytrace;
using namespace raytrace::objects;
using namespace raytrace::lights;
using namespace raytrace::colors;
using namespace raytrace::operators;
using namespace iso::literals;

/// A line of planets on the ground (XY plane, Z=0) receding into the distance.
/// Shows per-channel Rayleigh-like atmospheric extinction: blue scatters fastest, red scatters least.
class HazeWorld : public world {
public:
    HazeWorld()
        : world{raytrace::point{0, -70, 0}, raytrace::point{0, 500, 0}, "Atmospheric Haze", "world_haze.tga"}
        , sun_rays{raytrace::vector{-50, 100, -30}, colors::white, lights::intensities::full * 1.5}
        , dense_haze{mediums::refractive_index::air, color(0.002_p, 0.004_p, 0.008_p), colors::light_sky_blue}
        , s0{raytrace::point{-50, 0, 0}, 38}
        , s1{raytrace::point{-25, 100, 0}, 30}
        , s2{raytrace::point{0, 200, 0}, 30}
        , s3{raytrace::point{25, 300, 0}, 30}
        , s4{raytrace::point{50, 400, 0}, 30}
        , s5{raytrace::point{75, 500, 0}, 30}
        , s6{raytrace::point{100, 600, 0}, 30}
        , white_mtl{colors::white, mediums::ambient::dim, colors::white, mediums::smoothness::polished,
                    mediums::roughness::medium}
        , red_mtl{colors::red, mediums::ambient::dim, colors::red, mediums::smoothness::polished,
                  mediums::roughness::medium}
        , green_mtl{colors::green, mediums::ambient::dim, colors::green, mediums::smoothness::polished,
                    mediums::roughness::medium}
        , blue_mtl{colors::blue, mediums::ambient::dim, colors::blue, mediums::smoothness::polished,
                   mediums::roughness::medium} {
        s0.material(&white_mtl);
        s1.material(&red_mtl);
        s2.material(&green_mtl);
        s3.material(&blue_mtl);
        s4.material(&white_mtl);
        s5.material(&red_mtl);
        s6.material(&green_mtl);
    }

    ~HazeWorld() = default;

    raytrace::color background(raytrace::ray const& world_ray) const override {
        iso::radians sky_angle = angle(R3::basis::Z, world_ray.direction());
        precision scalar = std::clamp(sky_angle.value / iso::pi, 0.0_p, 1.0_p);
        return fourcc::linear::interpolate(colors::light_sky_blue, colors::grey, scalar);
    }

    void add_to(scene& scene) override {
        scene.add_light(&sun_rays);
        scene.add_object(&s0);
        scene.add_object(&s1);
        scene.add_object(&s2);
        scene.add_object(&s3);
        scene.add_object(&s4);
        scene.add_object(&s5);
        scene.add_object(&s6);
        scene.add_media(&dense_haze);
    }

    raytrace::animation::anchors get_anchors() const override {
        raytrace::animation::anchors anchors;
        anchors.push_back(
            animation::Anchor{animation::Attributes{look_from, look_at, iso::degrees{30.0_p}},
                              animation::Attributes{raytrace::point{50, -50, 50}, look_at, iso::degrees{45.0_p}},
                              animation::Mappers{}, iso::seconds{5.0_p}});
        return anchors;
    }

protected:
    raytrace::point look_from;
    raytrace::point look_at;
    raytrace::lights::beam sun_rays;
    // Stylized dense Rayleigh-like haze so the extinction is clearly visible at ~600 units.
    // earth_atmosphere itself is now physically realistic (subtle over these distances).
    raytrace::mediums::transparent dense_haze;
    raytrace::objects::sphere s0, s1, s2, s3, s4, s5, s6;
    raytrace::mediums::plain white_mtl, red_mtl, green_mtl, blue_mtl;
};

world* get_world() {
    static HazeWorld my_world;
    return &my_world;
}
