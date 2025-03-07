///
/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Renders a raytraced image of a bunch of spheres
/// @date 2021-01-02
/// @copyright Copyright (c) 2021
///

#include <raytrace/raytrace.hpp>

#include "world.hpp"

using namespace raytrace;
using namespace iso::literals;

constexpr static size_t number_of_spheres_per_side = 11;  // 7; // this has to be odd
constexpr static precision radius = 3.0_p;                // 4.5_p;
static_assert(number_of_spheres_per_side % 2 == 1, "number_of_spheres_per_side must be odd");

class MaterialWorld : public world {
public:
    MaterialWorld()
        : world{}
        , look_from{0, -20, 80}
        , look_at{0, 0, 0}
        , number_of_spheres{number_of_spheres_per_side * number_of_spheres_per_side}
        , brightness{lights::intensities::moderate}
        , spheres{}
        , mats{}
        , sunlight{raytrace::vector{0, -1, -1}, colors::white, brightness}
        , specks{} {
        size_t half_count = number_of_spheres_per_side / 2;
        for (size_t j = 0; j < number_of_spheres_per_side; j++) {
            for (size_t i = 0; i < number_of_spheres_per_side; i++) {
                raytrace::point start{0.0_p, 0.0_p, radius};
                // create the sphere where we are looking
                spheres.push_back(new raytrace::objects::sphere{start, radius});
                // index into each sphere uniquely
                size_t s = j * number_of_spheres_per_side + i;
                // generate the move vector
                precision x = ((precision)i - (precision)half_count) * 2 * radius;
                precision y = ((precision)j - (precision)half_count) * 2 * radius;
                raytrace::vector mover{x, y, 0.0_p};
                // move the sphere to the correct location
                spheres[s]->move_by(mover);
                // pick the color based on the position
                raytrace::color c = j % 2 == 0 ? colors::red : colors::green;
                // create a plain material for each sphere
                precision smooth = 1.0_p * (precision)i / (precision)number_of_spheres_per_side;
                precision rough = 1.0_p * (precision)j / (precision)number_of_spheres_per_side;
                auto* mat = new mediums::plain{c, 0.0_p, c, smooth, rough};
                // assign the material
                mats.push_back(mat);
                spheres[s]->material(mats[s]);
            }
        }
        // specks.push_back(new lights::speck(raytrace::point{-80, 0, 80}, colors::white, brightness));
        // specks.push_back(new lights::speck(raytrace::point{-40, 0, 80}, colors::white, brightness));
        // specks.push_back(new lights::speck(raytrace::point{40, 0, 80}, colors::white, brightness));
        // specks.push_back(new lights::speck(raytrace::point{80, 0, 80}, colors::white, brightness));
    }

    ~MaterialWorld() {
        for (auto& s : spheres) {
            delete s;
        }
        for (auto& s : mats) {
            delete s;
        }
        for (auto& s : specks) {
            delete s;
        }
    }

    raytrace::point& looking_from() override {
        return look_from;
    }

    raytrace::point& looking_at() override {
        return look_at;
    }

    std::string window_name() const override {
        return std::string("Plain Materials Smooth vs Rough");
    }

    std::string output_filename() const override {
        return std::string("world_plain_materials.tga");
    }

    raytrace::color background(raytrace::ray const& world_ray) const override {
        iso::radians A = angle(R3::basis::Z, world_ray.direction());
        precision B = A.value / iso::pi;
        return color(0.3_p * B, 0.3_p * B, 0.3_p * B);
    }

    void add_to(scene& scene) override {
        // add the objects to the scene.
        for (auto& s : spheres) {
            scene.add_object(s);
        }
        for (auto& s : specks) {
            scene.add_light(s);
        }
        scene.add_light(&sunlight);
    }

    raytrace::animation::anchors get_anchors() const override {
        raytrace::animation::anchors anchors;
        anchors.push_back(animation::Anchor{animation::Attributes{look_from, look_at, iso::degrees{55.0}},
                                            animation::Attributes{look_from, look_at, iso::degrees{55.0}},
                                            animation::Mappers{}, iso::seconds{1.0_p}});
        return anchors;
    }

protected:
    raytrace::point look_from;
    raytrace::point look_at;
    size_t number_of_spheres;
    precision brightness;
    std::vector<raytrace::objects::sphere*> spheres;
    std::vector<raytrace::mediums::plain*> mats;
    lights::beam sunlight;
    std::vector<raytrace::lights::speck*> specks;
};

// declare a single instance and return the reference to it
world* get_world() {
    static MaterialWorld my_world;
    return &my_world;
}
