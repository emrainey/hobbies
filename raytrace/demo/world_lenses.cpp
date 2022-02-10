/**
 * @file
 * @author Erik Rainey (erik.rainey@gmail.com)
 * @brief Renders a raytraced image of a lens with a grid background
 * @date 2022-01-02
 * @copyright Copyright (c) 2022
 */

#include <raytrace/raytrace.hpp>
#include "world.hpp"

using namespace raytrace;

class ConvexLensData {
public:
    ConvexLensData(element_type d, iso::radians a) {
        diameter = d;
        angle = a;
        radius = (diameter * 0.5) / std::sin(angle.value);
        separation = (diameter * 0.5) / std::tan(angle.value);
        bulge = radius - separation;
    }
    iso::radians angle;
    element_type diameter;
    element_type radius;
    element_type separation;
    element_type bulge;
};

class LensWorld : public world {
public:
    LensWorld()
        : world()
        , look_from(35, 0, 20)
        , look_at(0, 0, 0)
        , grid(1.0, colors::dark_olive_green, colors::yellow)
        , glass(refractive_index::glass, 0.88)
        , info(10.0, iso::radians(iso::pi/4))
        , x0(raytrace::point(info.separation, 0, 5), info.radius)
        , x1(raytrace::point(-info.separation, 0, 5), info.radius)
        , convex_lens(x0, x1, overlap::type::inclusive)
        , ground(R3::origin, R3::basis::Z, 1000, 1000)
        , sunlight(raytrace::vector{-2, 2, -1}, colors::white, 1E11)
        , prick(raytrace::point(0, 0, 12), colors::dim_grey, 1E3)
    {
        // reduce the volumetric point to a planar point
        grid.mapper(std::bind(&raytrace::square::map, &ground, std::placeholders::_1));

        x0.material(&glass);
        x1.material(&glass);
        convex_lens.material(&glass); // should assign all submaterials too? we assume const refs so it can't.
        ground.material(&grid);
    }

    ~LensWorld() {}

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
        return std::string("world_lenses.ppm");
    }

    raytrace::color background(const raytrace::ray& world_ray) const override {
        return colors::black;
    }

    void add_to(scene& scene) override {
        //scene.add_object(&base);
        scene.add_object(&convex_lens);
        //scene.add_object(&x0);
        //scene.add_object(&x1);
        scene.add_object(&ground);
        scene.add_light(&sunlight);
        scene.add_light(&prick);
    }

protected:
    raytrace::point look_from;
    raytrace::point look_at;
    raytrace::grid grid;
    raytrace::transparent glass;
    ConvexLensData info;
    raytrace::sphere x0;
    raytrace::sphere x1;
    raytrace::overlap convex_lens;
    raytrace::square ground;
    raytrace::beam sunlight;
    raytrace::speck prick;
};

// declare a single instance and return the reference to it
world* get_world() {
    static LensWorld my_world;
    return &my_world;
}

