#include <functional>
#include <raytrace/raytrace.hpp>

#include "world.hpp"

using namespace raytrace;
using namespace raytrace::objects;
using namespace raytrace::lights;
using namespace iso::literals;

class EmissiveWorld : public world {
public:
    EmissiveWorld()
        : world{raytrace::point{5, -14, 1}, raytrace::point{0, 0, 0}, "Emissive World", "world_emissive.tga"}
        , light_subsamples{20}
        , plain_white{colors::white, mediums::ambient::none, colors::white, mediums::smoothness::none, roughness::tight}
        , checkerboard_grid{2.0_p, colors::black, colors::white}
        , floor{30}
        , beam1{-R3::basis::Z, colors::white, 0.75_p}
        , s1{R3::point{1.0_p, -2.0_p, 3.0_p}, 2.0_p, 1.0_p, 2.0_p}
        , s2{R3::point{-2.0_p, -5.0_p, 2.5_p}, 1.3_p}
        , s3{R3::point{-4.0_p, 4.0_p, 9.0_p}, 2.0_p, 3.3_p, 8.7_p}
        , s4{R3::point{8.0_p, 5.0_p, 8.0_p}, 7.5_p}
        , s5{R3::point{0.0_p, 0.0_p, 1.0_p}, 0.5_p}
        , s6{R3::point{4.0_p, -2.0, 0.0_p}, 1.0_p, 2.0_p}
        , emissive_surface{R3::point{2.0_p, -3.0_p, 1.5_p}, 0.75_p}
        , glow{colors::black, colors::red, mediums::smoothness::none, mediums::roughness::tight} {
        checkerboard_grid.mapper(std::bind(&objects::square::map, &floor, std::placeholders::_1));
        floor.material(&checkerboard_grid);
        s1.material(&mediums::metals::stainless);
        s2.material(&mediums::metals::stainless);
        s3.material(&mediums::metals::stainless);
        s4.material(&mediums::metals::stainless);
        s5.material(&mediums::metals::stainless);
        s6.material(&mediums::metals::stainless);
        emissive_surface.material(&glow);
        s1.rotation(iso::radians{0}, iso::radians{0}, iso::radians{-iso::pi / 4});
    }

    ~EmissiveWorld() = default;

    raytrace::color background(raytrace::ray const& world_ray) const override {
        iso::radians A = angle(R3::basis::Z, world_ray.direction());
        precision B = A.value / iso::pi;
        return color(0.2_p * B, 0.2_p * B, 0.2_p * B);
    }

    void add_to(scene& scene) override {
        scene.add_light(&beam1);
        scene.add_object(&floor);
        scene.add_object(&s1);
        scene.add_object(&s2);
        scene.add_object(&s3);
        scene.add_object(&s4);
        scene.add_object(&s5);
        scene.add_object(&s6);
        scene.add_object(&emissive_surface);
    }

    raytrace::animation::anchors get_anchors() const override {
        raytrace::animation::anchors anchors;
        anchors.push_back(animation::Anchor{animation::Attributes{looking_from(), looking_at(), 37.0_deg},
                                            animation::Attributes{R3::point{-5, -14, 1}, looking_at(), 37.0_deg},
                                            animation::Mappers{}, iso::seconds{3.0_p}});
        anchors.push_back(animation::Anchor{anchors.back().limit,
                                            animation::Attributes{raytrace::point{-10, 10, 1}, looking_at(), 37.0_deg},
                                            animation::Mappers{}, iso::seconds{3.0_p}});
        return anchors;
    }

protected:
    size_t light_subsamples;
    mediums::plain plain_white;
    mediums::grid checkerboard_grid;
    raytrace::objects::square floor;
    raytrace::lights::beam beam1;
    raytrace::objects::ellipsoid s1;
    raytrace::objects::sphere s2;
    raytrace::objects::ellipsoid s3;
    raytrace::objects::sphere s4;
    raytrace::objects::sphere s5;
    raytrace::objects::ellipticalcylinder s6;
    raytrace::objects::sphere emissive_surface;
    raytrace::mediums::glowy glow;
};

// declare a single instance and return the reference to it
world* get_world() {
    static EmissiveWorld my_world;
    return &my_world;
}
