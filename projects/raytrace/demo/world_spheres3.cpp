///
/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Renders a raytraced image of a bunch of spheres in a halo around a central sphere.
/// @date 2021-01-02
/// @copyright Copyright (c) 2021
///

#include <raytrace/raytrace.hpp>
#include <noise/noise.hpp>

#include "world.hpp"

using namespace raytrace;
using namespace iso::literals;

using mat = const mediums::metal;

class Spheres3x3World : public world {
public:
    Spheres3x3World()
        : world{}
        , look_from{0, -30, 60}
        , look_at{0, 0, 0}
        , sunlight{raytrace::vector{1, 8, -8}, colors::white, lights::intensities::full}
        , dull{colors::white, 0.1_p, colors::red, mediums::smoothness::small, mediums::roughness::loose}
        , s1x1{raytrace::point{-15, 15, 5}, 5}
        , happy{2.0_p, colors::black, colors::yellow}
        , s1x2{raytrace::point{0, 15, 5}, 5}
        , marble{1024.0_p, 1.65_p, 1.0_p, 4.0_p, 256.0_p, 2.75_p, colors::black, colors::white}
        , s1x3{raytrace::point{15, 15, 5}, 5}
        , dots{7.0_p, colors::black, colors::white}
        , s2x1{raytrace::point{-15, 0, 5}, 5}
        , s2x2{raytrace::point{0, 0, 5}, 5}
        , perlin{iso::degrees{81}.value, 10.0_p, 245.4993546_p, colors::black, colors::white}
        , s2x3{raytrace::point{15, 0, 5}, 5}
        , stripes{7.0_p, colors::red, colors::white}
        , s3x1{raytrace::point{-15, -15, 5}, 5}
        , ikea_checkerboard{5, colors::yellow, colors::blue}
        , s3x2{raytrace::point{0, -15, 5}, 5}
        , ikea_voxelboard{0.5_p, colors::yellow, colors::blue}
        , s3x3{raytrace::point{15, -15, 5}, 5} {
        raytrace::point center = look_at;
        s1x1.material(&dull);
        happy.mapper(std::bind(&raytrace::objects::sphere::map, &s1x2, std::placeholders::_1));
        s1x2.material(&happy);
        marble.mapper(std::bind(&raytrace::objects::sphere::map, &s1x3, std::placeholders::_1));
        s1x3.material(&marble);
        dots.mapper(std::bind(&raytrace::objects::sphere::map, &s2x1, std::placeholders::_1));
        s2x1.material(&dots);
        s2x2.material(&mediums::metals::silver);
        perlin.mapper(std::bind(&raytrace::objects::sphere::map, &s2x3, std::placeholders::_1));
        s2x3.material(&perlin);
        stripes.mapper(std::bind(&raytrace::objects::sphere::map, &s3x1, std::placeholders::_1));
        s3x1.material(&stripes);
        ikea_checkerboard.mapper(std::bind(&raytrace::objects::sphere::map, &s3x2, std::placeholders::_1));
        s3x2.material(&ikea_checkerboard);
        // ikea_voxelboard.mapper(std::bind(&raytrace::objects::sphere::map, &s3x3, std::placeholders::_1));
        s3x3.material(&ikea_voxelboard);
    }

    ~Spheres3x3World() = default;

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
        return color(0.8_p * B, 0.8_p * B, 0.8_p * B);
    }

    void add_to(scene& scene) override {
        scene.add_light(&sunlight);
        scene.add_object(&s1x1);
        scene.add_object(&s1x2);
        scene.add_object(&s1x3);
        scene.add_object(&s2x1);
        scene.add_object(&s2x2);
        scene.add_object(&s2x3);
        scene.add_object(&s3x1);
        scene.add_object(&s3x2);
        scene.add_object(&s3x3);
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

    lights::beam sunlight;

    mediums::plain dull;
    objects::sphere s1x1;
    mediums::happy_face happy;
    objects::sphere s1x2;
    mediums::turbsin marble;
    objects::sphere s1x3;
    mediums::dots dots;
    objects::sphere s2x1;
    objects::sphere s2x2;  // silver mirror
    mediums::perlin perlin;
    objects::sphere s2x3;
    mediums::stripes stripes;
    objects::sphere s3x1;
    mediums::checkerboard ikea_checkerboard;
    objects::sphere s3x2;
    mediums::checkerboard ikea_voxelboard;
    objects::sphere s3x3;
};

// declare a single instance and return the reference to it
world* get_world() {
    static Spheres3x3World my_world;
    return &my_world;
}
