///
/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Renders a raytraced image of a box with a sphere cutout, then rounded by another sphere with a floating
/// sphere over top.
/// @date 2021-01-02
/// @copyright Copyright (c) 2022
///

#include <raytrace/raytrace.hpp>

#include "world.hpp"

using namespace raytrace;
using namespace iso::literals;

class DeskToyWorld : public world {
public:
    DeskToyWorld()
        : world{}
        , look_from{20, 20, 20}
        , look_at{0, 0, 0}
        , block{R3::origin, 8, 8, 2}           // center of the overlap
        , cutout{raytrace::point{0, 0, 4}, 4}  // overlap space centered at R3::origin
                                               // cutout from block, centered on block by definition
        , base{block, cutout, raytrace::objects::overlap::type::subtractive}
        , floater{raytrace::point{0, 0, 6}, 2.7}
        , cutout2{raytrace::point{0, 0, -17}, 20}
        // overlap space will be centered 1/2 way between origin and the cutout2
        // cutout2 from base. the center will now be at 0, 0, -7.5 due to centroid calculation
        , final_base{base, cutout2, raytrace::objects::overlap::type::inclusive}
        , ground{R3::origin, R3::basis::Z}
        , sunlight{raytrace::vector{-2, 2, -1}, colors::white, lights::intensities::bright}
        , prick{raytrace::point{0, 0, 3}, colors::white, lights::intensities::bright} {
        // the final base should sit on the ground
        // base.position(raytrace::point{0, 0, 2});
        final_base.position(raytrace::point{0, 0, 2});
        block.material(&mediums::metals::stainless);
        cutout.material(&mediums::metals::stainless);
        cutout2.material(&mediums::metals::stainless);
        floater.material(&mediums::metals::stainless);
        ground.material(&mediums::metals::copper);
        final_base.material(&mediums::metals::stainless);
    }

    ~DeskToyWorld() {
    }

    raytrace::point& looking_from() override {
        return look_from;
    }

    raytrace::point& looking_at() override {
        return look_at;
    }

    std::string window_name() const override {
        return std::string("DeskToy");
    }

    std::string output_filename() const override {
        return std::string("world_desktoy.tga");
    }

    raytrace::color background(raytrace::ray const& world_ray) const override {
        iso::radians A = angle(R3::basis::Z, world_ray.direction());
        precision B = A.value / iso::pi;
        using namespace raytrace::operators;
        return colors::grey * B;
    }

    void add_to(scene& scene) override {
        scene.add_object(&floater);
        scene.add_object(&ground);
        // scene.add_object(&base);
        scene.add_object(&final_base);
        scene.add_light(&sunlight);
        // scene.add_light(&prick);
    }

    raytrace::animation::anchors get_anchors() const override {
        raytrace::animation::anchors anchors;
        anchors.push_back(animation::Anchor{animation::Attributes{look_from, look_at, 55.0_deg},
                                            animation::Attributes{raytrace::point{20, 0, 20}, look_at, 55.0_deg},
                                            animation::Mappers{}, iso::seconds{4.0_p}});
        anchors.push_back(animation::Anchor{anchors.back().limit,
                                            animation::Attributes{raytrace::point{0, 20, 20}, look_at, 55.0_deg},
                                            animation::Mappers{}, iso::seconds{4.0_p}});
        return anchors;
    }

protected:
    raytrace::point look_from;
    raytrace::point look_at;
    raytrace::objects::cuboid block;
    raytrace::objects::sphere cutout;
    raytrace::objects::overlap base;
    raytrace::objects::sphere floater;
    raytrace::objects::sphere cutout2;
    raytrace::objects::overlap final_base;
    raytrace::objects::plane ground;
    raytrace::lights::beam sunlight;
    raytrace::lights::speck prick;
};

// declare a single instance and return the reference to it
world* get_world() {
    static DeskToyWorld my_world;
    return &my_world;
}
