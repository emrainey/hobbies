///
/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Renders an example raytraced image.
/// @date 2020-05-24
/// @copyright Copyright (c) 2020
///

#include <raytrace/raytrace.hpp>

#include "world.hpp"

using namespace raytrace;
using namespace raytrace::objects;
using namespace raytrace::lights;
using namespace iso::literals;

class ExampleWorld : public world {
public:
    ExampleWorld()
        : world{}
        , look_from{-50, -50, 50}
        , look_at{0, 0, 0}
        , tilt{rotation(iso::radians(0), iso::radians(0), iso::radians(iso::pi / 5))}
        , plain_yellow{colors::yellow, mediums::ambient::none, colors::yellow, mediums::smoothness::small,
                       mediums::roughness::tight}
        , plain_blue{colors::white, mediums::ambient::none, colors::blue, mediums::smoothness::small,
                     mediums::roughness::tight}
        , plain_red{colors::white, mediums::ambient::none, colors::red, mediums::smoothness::small,
                    mediums::roughness::tight}
        , plain_cyan{colors::white, mediums::ambient::none, colors::cyan, mediums::smoothness::small,
                     mediums::roughness::tight}
        , grey_checkers{3, colors::grey, colors::red}
        , ikea_checkers{5, colors::yellow, colors::blue}
        , polka_dots{3.0_p, colors::black, colors::white}
        , bw_marble{1024.0_p, 1.675_p, 1.0_p, 6.0_p, 256.0_p, 4.75_p, colors::black, colors::white}
        , candy_stripes{1.0_p, colors::red, colors::white}
        , grid1{10.0_p, colors::green, colors::black}
        , checker_ball_radius{7.5_p}
        , checker_ball_ring_radius_inner{checker_ball_radius + 2.0_p}
        , checker_ball_ring_radius_outer{checker_ball_ring_radius_inner + 9.0_p}
        , checker_ball{raytrace::point{0, 30, checker_ball_radius}, checker_ball_radius}
        , checker_ball_ring0{checker_ball.position(), tilt, checker_ball_ring_radius_inner,
                             checker_ball_ring_radius_outer}
        , checker_ball_ring1{checker_ball.position(), tilt * -1, checker_ball_ring_radius_inner,
                             checker_ball_ring_radius_outer}
        , cyl_position{-30, 00, 10}
        , top{cyl_position + R3::vector{{0, 0, 10}}}
        , cap{top, R3::identity, 0, 10}
        , column{cyl_position, 10, 10}
        , polka_dot_cube{raytrace::point{30, 0, 7.5_p}, 7.5_p, 7.5_p, 7.5_p}
        , ground{}
        , cone_position{0, 0, 10}
        , inner_cone{cone_position, iso::radians(iso::pi / 12)}
        , bounding_cone{cone_position, 10, 10, 10}
        , overlapped_cone{inner_cone, bounding_cone, overlap::type::inclusive}
        , torus1{raytrace::point{0, 0, 10}, 7, 2}
        , pyramid{raytrace::point{0, -30, 10}, 10}
        , white_light{raytrace::point{20, 20, 20}, colors::white, lights::intensities::full}
        , sunlight{raytrace::vector{0, 1, -1}, colors::white, lights::intensities::full} {
        // assign materials in the body of the constructor!

        // 2d mapping instead of 3d
        ikea_checkers.mapper(std::bind(&raytrace::objects::sphere::map, &checker_ball, std::placeholders::_1));
        checker_ball.material(&ikea_checkers);
        // 2d mapping instead of 3d
        checker_ball_ring0.material(&plain_blue);
        checker_ball_ring1.material(&plain_yellow);
        // 2d mapping instead of 3d
        candy_stripes.mapper(std::bind(&raytrace::objects::cylinder::map, &column, std::placeholders::_1));
        column.material(&candy_stripes);
        cap.material(&candy_stripes);

        // 2d mapping instead of 3d
        polka_dots.mapper(std::bind(&raytrace::objects::cuboid::map, &polka_dot_cube, std::placeholders::_1));
        polka_dot_cube.material(&polka_dots);

        // plane is already 2d, no mapping needed
        grid1.mapper(std::bind(&raytrace::objects::plane::map, &ground, std::placeholders::_1));
        ground.material(&grid1);
        ground.set_surface_scale(1.0_p, 1.0_p / 32);

        inner_cone.material(&plain_red);
        bounding_cone.material(&plain_red);
        overlapped_cone.material(&plain_red);

        // 2d mapping instead of 3d
        grey_checkers.mapper(std::bind(&raytrace::objects::torus::map, &torus1, std::placeholders::_1));
        torus1.material(&grey_checkers);

        pyramid.material(&plain_cyan);

        // assign any rotation or translations

        // checker_ball.rotation(iso::radians{0}, iso::radians{0}, iso::radians{iso::pi});
        // polka_dot_cube.rotation(iso::degrees(45), iso::degrees(45), iso::degrees(45));
    }

    ~ExampleWorld() = default;

    raytrace::point& looking_from() override {
        return look_from;
    }

    raytrace::point& looking_at() override {
        return look_at;
    }

    std::string window_name() const override {
        return std::string("Raytracing Example");
    }

    std::string output_filename() const override {
        return std::string("world_example.tga");
    }

    raytrace::color background(raytrace::ray const&) const override {
        return colors::black;
    }

    void add_to(scene& scene) override {
        // add the objects to the scene.
        scene.add_object(&ground);
        scene.add_object(&polka_dot_cube);
        scene.add_object(&column);
        scene.add_object(&cap);
        // scene.add_object(&inner_cone);
        scene.add_object(&overlapped_cone);
        scene.add_object(&checker_ball);
        scene.add_object(&checker_ball_ring0);
        scene.add_object(&checker_ball_ring1);
        scene.add_object(&torus1);
        scene.add_object(&pyramid);
        // add lights to the scene
        scene.add_light(&sunlight);
        scene.add_light(&white_light);
    }

    raytrace::animation::anchors get_anchors() const override {
        raytrace::animation::anchors anchors;
        anchors.push_back(animation::Anchor{animation::Attributes{look_from, look_at, 55.0_deg},
                                            animation::Attributes{raytrace::point{-50, 50, 30}, look_at, 23.0_deg},
                                            animation::Mappers{}, iso::seconds{10.0_p}});
        anchors.push_back(animation::Anchor{anchors.back().limit,  // previous limit is this start
                                            animation::Attributes{raytrace::point{50, -50, 30}, look_at, 23.0_deg},
                                            animation::Mappers{}, iso::seconds{10.0_p}});
        return anchors;
    }

protected:
    raytrace::point look_from;
    raytrace::point look_at;
    matrix tilt;
    // define some surfaces
    plain plain_yellow;
    plain plain_blue;
    plain plain_red;
    plain plain_cyan;
    checkerboard grey_checkers;
    checkerboard ikea_checkers;
    dots polka_dots;
    turbsin bw_marble;
    stripes candy_stripes;
    grid grid1;

    // sphere w/ ring
    precision checker_ball_radius;
    precision checker_ball_ring_radius_inner;
    precision checker_ball_ring_radius_outer;
    raytrace::point checker_ball_position;
    raytrace::objects::sphere checker_ball;
    ring checker_ball_ring0;
    ring checker_ball_ring1;

    raytrace::point cyl_position;
    raytrace::point top;
    ring cap;
    cylinder column;

    cuboid polka_dot_cube;
    raytrace::objects::plane ground;
    raytrace::point pos5;
    raytrace::point pos6;
    raytrace::point pos7;

    raytrace::point cone_position;
    cone inner_cone;
    cuboid bounding_cone;
    overlap overlapped_cone;

    torus torus1;
    pyramid pyramid;

    // create some lights
    speck white_light;
    beam sunlight;
};

// declare a single instance and return the reference to it
world* get_world() {
    static ExampleWorld my_world;
    return &my_world;
}
