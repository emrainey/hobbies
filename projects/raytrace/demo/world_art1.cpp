#include <raytrace/raytrace.hpp>

#include "world.hpp"

using namespace raytrace;
using namespace raytrace::objects;
using namespace raytrace::lights;
using namespace raytrace::colors;
using namespace raytrace::operators;
using namespace iso::literals;

// Good results at values of depth=8, subsamples=16, 1024x768, fov=23
class ArtWorld : public world {
public:
    ArtWorld()
        : world(raytrace::point{25, 25, 10}, raytrace::point{0, 0, 10}, "Art1 World", "world_art1.tga")
        , sun_rays{raytrace::vector{-20, 0, -21}, colors::white, lights::intensities::full + 0.5_p}
        , floor{2000.0_p}
        , pyramid1{origin, 10.0_p}
        , orb{raytrace::point{0, 0, 12}, 2.0_p}
        , halo{raytrace::point{0, 0, 14}, 2.25_p, 0.125_p}
        , columns{} {
        // four columns around the pyramid
        precision height = 10.0_p;
        columns.emplace_back(raytrace::point{12, 0, height / 2}, height, 0.5_p);
        columns.emplace_back(raytrace::point{-12, 0, height / 2}, height, 0.5_p);
        columns.emplace_back(raytrace::point{0, 12, height / 2}, height, 0.5_p);
        columns.emplace_back(raytrace::point{0, -12, height / 2}, height, 0.5_p);

        pyramid1.material(&mediums::metals::stainless);
        orb.material(&mediums::metals::stainless);
        floor.material(&mediums::metals::stainless);
        for (auto& column : columns) {
            column.material(&mediums::metals::stainless);
        }
        halo.material(&mediums::metals::copper);
        ambient_ = colors::stainless;
        // this is the only way to set the intensity of the ambient light is in the I channel
        ambient_.intensity(0.75_p);
    }

    ~ArtWorld() = default;

    raytrace::color background(raytrace::ray const& world_ray) const override {
        // this creates a gradient from top to bottom
        iso::radians sky_angle = angle(R3::basis::Z, world_ray.direction());
        precision scalar = sky_angle.value / iso::pi;
        return fourcc::linear::interpolate(colors::dark_slate_blue, colors::light_blue, scalar);
        // return colors::black;
    }

    void add_to(scene& scene) override {
        // scene.add_light(&inner_light);
        scene.add_light(&sun_rays);
        scene.add_object(&floor);
        scene.add_object(&pyramid1);
        scene.add_object(&orb);
        scene.add_object(&halo);
        for (auto& column : columns) {
            scene.add_object(&column);
        }
        scene.add_media(&mediums::earth_atmosphere);
    }

    raytrace::animation::anchors get_anchors() const override {
        raytrace::animation::anchors anchors;
        anchors.push_back(
            animation::Anchor{animation::Attributes{looking_from(), looking_at(), iso::degrees{23.0_p}},
                              animation::Attributes{raytrace::point{25, 0, 10}, looking_at(), iso::degrees{55.0_p}},
                              animation::Mappers{}, iso::seconds{5.0_p}});
        anchors.push_back(
            animation::Anchor{anchors.back().limit,  // previous limit is this start
                              animation::Attributes{raytrace::point{0, -25, 10}, looking_at(), iso::degrees{80.0_p}},
                              animation::Mappers{}, iso::seconds{5.0_p}});
        anchors.push_back(
            animation::Anchor{anchors.back().limit,  // previous limit is this start
                              animation::Attributes{raytrace::point{-25, 0, 10}, looking_at(), iso::degrees{55.0_p}},
                              animation::Mappers{}, iso::seconds{5.0_p}});
        return anchors;
    }

protected:
    lights::beam sun_rays;
    raytrace::point center;
    raytrace::objects::square floor;
    raytrace::objects::pyramid pyramid1;
    raytrace::objects::sphere orb;
    raytrace::objects::torus halo;
    std::vector<raytrace::objects::cylinder> columns;
};

// declare a single instance and return the reference to it
world* get_world() {
    static ArtWorld my_world;
    return &my_world;
}
