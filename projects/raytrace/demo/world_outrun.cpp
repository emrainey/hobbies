#include <raytrace/raytrace.hpp>

#include "world.hpp"

using namespace raytrace;
using namespace raytrace::objects;
using namespace raytrace::lights;
using namespace raytrace::colors;
using namespace raytrace::operators;

namespace outrun {
constexpr color neon_orange(1.0, 108.0 / 255, 17.0 / 255);
constexpr color neon_pink(246.0 / 255, 1.0 / 255, 157.0 / 255);

class Sun : public raytrace::mediums::opaque {
public:
    Sun() : opaque{} {
        m_ambient = colors::white;
        m_ambient_scale = 0.0;
        m_smoothness = smoothness::none;
        m_tightness = roughness::loose;
    }

    color diffuse(const raytrace::point& volumetric_point) const override {
        return colors::black;
    }

    color specular(const raytrace::point& volumetric_point, precision scaling,
                   const color& light_color) const override {
        return colors::black;
    }

    color emissive(const raytrace::point& volumetric_point) const override {
        raytrace::vector obj_vec = volumetric_point - R3::origin;
        iso::radians obj_angle = angle(R3::basis::Z, obj_vec);
        precision scalar = obj_angle.value / (iso::pi / 2.0);
        if (scalar < 1.0) {
            return interpolate(colors::orange_red, colors::yellow, scalar);
        } else {
            if ((1.0 < scalar and scalar <= 1.05) or (1.1 < scalar and scalar <= 1.15)
                or (1.2 < scalar and scalar <= 1.25) or (1.3 < scalar and scalar <= 1.35)) {
                return colors::black;
            } else {
                return interpolate(colors::red, colors::orange_red, 1.0 - scalar);
            }
        }
    }

    void radiosity(const raytrace::point& volumetric_point, precision refractive_index,
                   const iso::radians& incident_angle, const iso::radians& transmitted_angle, precision& emitted,
                   precision& reflected, precision& transmitted) const override {
        emitted = 1.0;
        reflected = 1.0 - m_transmissivity;
        transmitted = m_transmissivity;
    }
};
}  // namespace outrun

// Good results at values of depth=8, subsamples=16, 1024x768, fov=23
class OutrunWorld : public world {
public:
    OutrunWorld()
        : look_from{0, 50, 10}
        , look_at{0, 0, 10}
        , sunrays{raytrace::vector{-20, 0, -21}, colors::white, 1E4}
        , grid{10.0, outrun::neon_pink, colors::black}
        , floor{R3::origin, R3::basis::Z, 100.0, 100.0}
        , sun_surface{}
        , sun{raytrace::point{0, -300, 50}, 100.0} {
        grid.mapper(std::bind(&raytrace::objects::square::map, &floor, std::placeholders::_1));
        floor.material(&grid);
        sun.material(&sun_surface);
    }

    ~OutrunWorld() = default;

    raytrace::point& looking_from() override {
        return look_from;
    }

    raytrace::point& looking_at() override {
        return look_at;
    }

    std::string window_name() const override {
        return std::string("Outrun World");
    }

    std::string output_filename() const override {
        return std::string("world_outrun.tga");
    }

    raytrace::color background(const raytrace::ray& world_ray) const override {
        // this creates a gradient from top to bottom
        iso::radians sky_angle = angle(R3::basis::Z, world_ray.direction());
        precision scalar = sky_angle.value / (2 * iso::pi);
        return interpolate(colors::dark_slate_blue, colors::black, scalar);
        // return colors::black;
    }

    void add_to(scene& scene) override {
        // scene.add_light(&inner_light);
        scene.add_light(&sunrays);
        scene.add_object(&floor);
        scene.add_object(&sun);
        scene.add_media(&mediums::earth_atmosphere);
    }

protected:
    raytrace::point look_from;
    raytrace::point look_at;
    lights::beam sunrays;
    raytrace::point center;
    raytrace::mediums::grid grid;
    raytrace::objects::square floor;
    outrun::Sun sun_surface;
    raytrace::objects::sphere sun;
};

// declare a single instance and return the reference to it
world* get_world() {
    static OutrunWorld my_world;
    return &my_world;
}
