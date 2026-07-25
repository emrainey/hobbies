///
/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Renders a raytraced image of spheres demonstrating roughness and phong shading
/// @date 2021-01-02
/// @copyright Copyright (c) 2021
///
#include <raytrace/raytrace.hpp>

#include "world.hpp"

using namespace raytrace;
using namespace iso::literals;

class Spheres1World : public world {
public:
    Spheres1World()
        : world{raytrace::point{0, -20, 20}, raytrace::point{0, 0, 0}, "Spheres 1 - Roughness/Phong Demo", "world_spheres1.tga"}
        , sunlight{raytrace::vector{-2, 3, -2}, colors::white, lights::intensities::full}
        , spheres{}
        , materials{} {
        // Create a grid of spheres with varying roughness and phong properties
        const size_t rows = 8;
        const size_t cols = 8;
        const precision spacing = 3.0_p;
        const precision radius = 1.2_p;
        
        // Create spheres in a grid pattern
        for (size_t r = 0; r < rows; ++r) {
            for (size_t c = 0; c < cols; ++c) {
                raytrace::point center{(c - (cols-1)/2.0_p) * spacing, (r - (rows-1)/2.0_p) * spacing, 0};
                spheres.push_back(new raytrace::objects::sphere(center, radius));
            }
        }
        
        // Create a base material and vary only the roughness and phong properties
        // Using the same material type with different parameters to show the range
        const color base_color = colors::blue;
        const precision base_reflectivity = 0.1_p;
        const precision base_diffuse = 0.8_p;
        
        // Create materials with varying roughness values in steps of 0.125
        // Roughness goes from 0.0 (smooth) to 1.0 (rough) in 0.125 steps
        // Phong (smoothness) goes from 0.0 (matte) to 1.0 (shiny) in 0.125 steps
        precision roughness_step = 0.125_p;
        precision smoothness_step = 0.125_p;
        
        for (size_t r = 0; r < rows; ++r) {
            for (size_t c = 0; c < cols; ++c) {
                precision roughness = r * roughness_step;
                precision smoothness = c * smoothness_step;
                // Clamp values to valid range
                roughness = std::clamp(roughness, 0.0_p, 1.0_p);
                smoothness = std::clamp(smoothness, 0.0_p, 1.0_p);
                
                materials.push_back(new raytrace::mediums::plain(base_color, base_reflectivity, base_color, smoothness, roughness));
            }
        }
        
        // Assign materials to spheres
        for (size_t i = 0; i < spheres.size(); ++i) {
            spheres[i]->material(materials[i]);
        }
    }

    ~Spheres1World() {
        for (auto& s : spheres) {
            delete s;
        }
        for (auto& m : materials) {
            delete m;
        }
    }

    raytrace::color background(raytrace::ray const& world_ray) const override {
        iso::radians A = angle(R3::basis::Z, world_ray.direction());
        precision B = A.value / iso::pi;
        return color(0.8_p * B, 0.8_p * B, 0.8_p * B);
    }

    void add_to(scene& scene) override {
        // add the objects to the scene.
        for (auto& s : spheres) {
            scene.add_object(s);
        }
        scene.add_light(&sunlight);
    }

    raytrace::animation::anchors get_anchors() const override {
        raytrace::animation::anchors anchors;
        anchors.push_back(animation::Anchor{animation::Attributes{looking_from(), looking_at(), 55.0_deg},
                                            animation::Attributes{looking_from(), looking_at(), 55.0_deg},
                                            animation::Mappers{}, iso::seconds{1.0_p}});
        return anchors;
    }

protected:
    lights::beam sunlight;
    std::vector<raytrace::objects::sphere*> spheres;
    std::vector<raytrace::mediums::medium*> materials;
};

// declare a single instance and return the reference to it
world* get_world() {
    static Spheres1World my_world;
    return &my_world;
}