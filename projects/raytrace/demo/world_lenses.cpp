///
/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Renders a raytraced image of a lens with a grid background
/// @date 2022-01-02
/// @copyright Copyright (c) 2022
///

#include <raytrace/raytrace.hpp>

#include "world.hpp"

using namespace raytrace;
using namespace iso::literals;

class ConvexLensData {
public:
    ConvexLensData(precision d, iso::radians a) {
        diameter = d;
        angle = a;
        radius = (diameter * 0.5) / std::sin(angle.value);
        separation = (diameter * 0.5) / std::tan(angle.value);
        bulge = radius - separation;
    }
    iso::radians angle;
    precision diameter;
    precision radius;
    precision separation;
    precision bulge;
};

class LensWorld : public world {
public:
    LensWorld()
        : world{}
        , look_from{30, 0, 22}
        , look_at{0, 0, 0}
        , grid{1.0, colors::dark_olive_green, colors::yellow}
        , glass{mediums::refractive_index::glass, 0.22, colors::dark_gray}
        , info{10.0, iso::radians(iso::pi / 4)}
        , x0{raytrace::point{info.separation, 0, 5}, info.radius}
        , x1{raytrace::point{-info.separation, 0, 5}, info.radius}
        , convex_lens{x0, x1, objects::overlap::type::inclusive}
        , pyramid_center{10, 10, 10.1}
        , pyramid_core{pyramid_center, 10.1}
        , pyramid_bounds{pyramid_center, 5, 5, 10}
        // , pyramid_overlap{pyramid_core, pyramid_bounds, objects::overlap::type::inclusive}
        , glass_donut{raytrace::point{10, -10, 3.5}, 5.0, 2.5}
        , ground{R3::origin, R3::basis::Z, 1000, 1000}
        , sunlight{raytrace::vector{-2, 2, -1}, colors::white, lights::intensities::full}
        , prick{raytrace::point{0, 0, 22}, colors::white, 1E6} {
        // reduce the volumetric point to a planar point
        grid.mapper(std::bind(&raytrace::objects::square::map, &ground, std::placeholders::_1));
        look_at = raytrace::point{0, 0, info.radius};
        x0.material(&glass);
        x1.material(&glass);
        convex_lens.material(&glass);  // should assign all submaterials too? we assume const refs so it can't.
        // pyramid_core.material(&mediums::metals::chrome);
        // pyramid_bounds.material(&mediums::metals::chrome);
        // pyramid_overlap.material(&glass);
        glass_donut.material(&glass);
        ground.material(&grid);
    }

    ~LensWorld() = default;

    raytrace::point& looking_from() override {
        return look_from;
    }

    raytrace::point& looking_at() override {
        return look_at;
    }

    std::string window_name() const override {
        return std::string("Lenses");
    }

    std::string output_filename() const override {
        return std::string("world_lenses.tga");
    }

    raytrace::color background(raytrace::ray const& world_ray) const override {
        return colors::black;
    }

    void add_to(scene& scene) override {
        // scene.add_object(&base);
        scene.add_object(&convex_lens);
        // scene.add_object(&x0);
        // scene.add_object(&x1);
        // scene.add_object(&pyramid_core);
        // scene.add_object(&pyramid_bounds);
        // scene.add_object(&pyramid_overlap);
        scene.add_object(&glass_donut);
        scene.add_object(&ground);
        scene.add_light(&sunlight);
        scene.add_light(&prick);
    }

    raytrace::animation::anchors get_anchors() const override {
        raytrace::animation::anchors anchors;
        anchors.push_back(
            animation::Anchor{
                animation::Attributes{look_from, look_at, 55.0_deg},
                animation::Attributes{look_from, look_at, 55.0_deg},
                animation::Mappers{}, iso::seconds{1.0_p}
            });
        return anchors;
    }

protected:
    raytrace::point look_from;
    raytrace::point look_at;
    raytrace::mediums::grid grid;
    raytrace::mediums::transparent glass;
    ConvexLensData info;
    raytrace::objects::sphere x0;
    raytrace::objects::sphere x1;
    raytrace::objects::overlap convex_lens;

    raytrace::point pyramid_center;
    raytrace::objects::pyramid pyramid_core;
    raytrace::objects::cuboid pyramid_bounds;
    // raytrace::objects::overlap pyramid_overlap;

    raytrace::objects::torus glass_donut;

    raytrace::objects::square ground;
    raytrace::lights::beam sunlight;
    raytrace::lights::speck prick;
};

// declare a single instance and return the reference to it
world* get_world() {
    static LensWorld my_world;
    return &my_world;
}
