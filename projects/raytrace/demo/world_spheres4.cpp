#include <raytrace/raytrace.hpp>

#include "world.hpp"

using namespace raytrace;
using namespace raytrace::objects;
using namespace raytrace::lights;
using namespace raytrace::colors;
using namespace raytrace::operators;
using namespace iso::literals;

// Good results at values of depth=8, subsamples=16, 1024x768, fov=23
class SphereStackWorld : public world {
public:
    SphereStackWorld()
        : world{raytrace::point{0, 50, 20}, raytrace::point{0, 0, 10}, "A pile of spheres World", "world_spheres4.tga"}
        , sun_rays{raytrace::vector{-20, 0, -21}, colors::white, lights::intensities::full * 1.3}
        , floor{200.0_p}
        , cb0{0.125_p, colors::black, colors::white}
        , s0{raytrace::point{5.0_p, 0, 5}, 5.0_p}
        , s1{raytrace::point{-5.0_p, 0, 5}, 5.0_p}
        , s2{raytrace::point{0, sqrt(3.0_p) * 5, 5}, 5.0_p}
        , s3{raytrace::point{0, sqrt(3.0_p) * 5 / 2, 15}, 5.0_p}
        , reddish{colors::red, mediums::ambient::dim, colors::red, mediums::smoothness::none, mediums::roughness::tight}
        , greenish{colors::green, mediums::ambient::dim, colors::green, mediums::smoothness::none,
                   mediums::roughness::tight}
        , blueish{colors::blue, mediums::ambient::dim, colors::blue, mediums::smoothness::none,
                  mediums::roughness::tight}
        , yellowish{colors::yellow, mediums::ambient::dim, colors::yellow, mediums::smoothness::none,
                    mediums::roughness::tight} {
        cb0.mapper(std::bind(&raytrace::objects::plane::map, &floor, std::placeholders::_1));
        floor.material(&cb0);
        s0.material(&reddish);
        s1.material(&greenish);
        s2.material(&blueish);
        s3.material(&yellowish);
        s3.material(&mediums::metals::chrome);
    }

    ~SphereStackWorld() = default;

    raytrace::color background(raytrace::ray const& world_ray) const override {
        // this creates a gradient from top to bottom
        iso::radians sky_angle = angle(R3::basis::Z, world_ray.direction());
        precision scalar = sky_angle.value / iso::pi;
        return interpolate(colors::black, colors::white, scalar);
        // return colors::black;
    }

    void add_to(scene& scene) override {
        scene.add_light(&sun_rays);
        scene.add_object(&floor);
        scene.add_object(&s0);
        scene.add_object(&s1);
        scene.add_object(&s2);
        scene.add_object(&s3);
        scene.add_media(&mediums::earth_atmosphere);
    }

    raytrace::animation::anchors get_anchors() const override {
        raytrace::animation::anchors anchors;
        anchors.push_back(
            animation::Anchor{animation::Attributes{look_from, look_at, iso::degrees{23.0_p}},
                              animation::Attributes{raytrace::point{25, 0, 10}, look_at, iso::degrees{55.0_p}},
                              animation::Mappers{}, iso::seconds{5.0_p}});
        anchors.push_back(
            animation::Anchor{anchors.back().limit,  // previous limit is this start
                              animation::Attributes{raytrace::point{0, -25, 10}, look_at, iso::degrees{80.0_p}},
                              animation::Mappers{}, iso::seconds{5.0_p}});
        anchors.push_back(
            animation::Anchor{anchors.back().limit,  // previous limit is this start
                              animation::Attributes{raytrace::point{-25, 0, 10}, look_at, iso::degrees{55.0_p}},
                              animation::Mappers{}, iso::seconds{5.0_p}});
        return anchors;
    }

protected:
    raytrace::point look_from;
    raytrace::point look_at;
    raytrace::lights::beam sun_rays;
    raytrace::point center;
    raytrace::objects::square floor;
    raytrace::mediums::checkerboard cb0;
    raytrace::objects::sphere s0;
    raytrace::objects::sphere s1;
    raytrace::objects::sphere s2;
    raytrace::objects::sphere s3;
    raytrace::mediums::plain reddish;
    raytrace::mediums::plain greenish;
    raytrace::mediums::plain blueish;
    raytrace::mediums::plain yellowish;
};

// declare a single instance and return the reference to it
world* get_world() {
    static SphereStackWorld my_world;
    return &my_world;
}
