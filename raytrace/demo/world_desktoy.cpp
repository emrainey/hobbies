/**
 * @file
 * @author Erik Rainey (erik.rainey@gmail.com)
 * @brief Renders a raytraced image of a cylinder with another cylinder cut out
 * @date 2021-01-02
 * @copyright Copyright (c) 2022
 */

#include <raytrace/raytrace.hpp>
#include <basal/options.hpp>
#include "world.hpp"

using namespace raytrace;

class DesktoyWorld : public world {
public:
    DesktoyWorld()
        : world()
        , look_from(35, 35, 35)
        , look_at(0, 0,0)
        , steel(colors::grey, smoothness::polished, roughness::loose)
        , block(raytrace::point(0, 0, 2), 8, 8, 2)
        , cutout(raytrace::point(0, 0, 6), 4)
        , base(block, cutout, raytrace::overlap::type::subtractive) // cutout from block
        , floater(raytrace::point(0, 0, 6), 2.7)
        , cutout2(raytrace::point(0, 0, -15), 20)
        , final_base(base, cutout2, raytrace::overlap::type::inclusive) // cutout2 from base // FIXME not working
        , ground(R3::origin, R3::basis::Z, 1)
        , sunlight(raytrace::vector{-2, 2, -1}, colors::white, 1E11)
        , prick(raytrace::point(0, 0, 3), colors::dim_grey, 1E3)
    {
        block.material(&steel);
        cutout.material(&steel);
        cutout2.material(&steel);
        floater.material(&steel);
        ground.material(&steel);
    }

    ~DesktoyWorld() {}

    raytrace::point& looking_from() override {
        return look_from;
    }

    raytrace::point& looking_at() override {
        return look_at;
    }

    std::string window_name() const override {
        return std::string("Desktoy");
    }

    std::string output_filename() const override {
        return std::string("world_desktoy.ppm");
    }

    raytrace::color background(const raytrace::ray& world_ray) const override {
        iso::radians A = angle(R3::basis::Z, world_ray.direction());
        element_type B = A.value / iso::pi;
        using namespace raytrace::operators;
        return colors::grey * B;
    }

    void add_to(scene& scene) override {
        //scene.add_object(&base);
        scene.add_object(&floater);
        scene.add_object(&ground);
        scene.add_object(&final_base);
        scene.add_light(&sunlight);
        scene.add_light(&prick);
    }

protected:
    raytrace::point look_from;
    raytrace::point look_at;
    raytrace::metal steel;
    raytrace::cuboid block;
    raytrace::sphere cutout;
    raytrace::overlap base;
    raytrace::sphere floater;
    raytrace::sphere cutout2;
    raytrace::overlap final_base;
    raytrace::plane ground;
    raytrace::beam sunlight;
    raytrace::speck prick;
};

// declare a single instance and return the reference to it
world* get_world() {
    static DesktoyWorld my_world;
    return &my_world;
}

