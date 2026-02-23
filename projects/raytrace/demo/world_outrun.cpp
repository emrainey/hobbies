#include <raytrace/raytrace.hpp>

#include "world.hpp"

using namespace raytrace;
using namespace raytrace::objects;
using namespace raytrace::lights;
using namespace raytrace::colors;
using namespace raytrace::operators;
using namespace iso::literals;

namespace outrun {
constexpr color neon_orange(1.0_p, 108.0_p / 255, 17.0_p / 255);
constexpr color neon_pink(246.0_p / 255, 1.0_p / 255, 157.0_p / 255);

class Sun : public raytrace::mediums::opaque {
public:
    Sun() : opaque{} {
        m_ambient = colors::white;
        m_ambient_scale = 0.0_p;
        m_smoothness = smoothness::none;
        m_tightness = roughness::loose;
    }

    color diffuse(raytrace::point const& volumetric_point) const override {
        return colors::black;
    }

    color specular(raytrace::point const& volumetric_point, precision scaling,
                   color const& light_color) const override {
        return colors::black;
    }

    color emissive(raytrace::point const& volumetric_point) const override {
        raytrace::vector obj_vec = volumetric_point - R3::origin;
        iso::radians obj_angle = angle(R3::basis::Z, obj_vec);
        precision scalar = obj_angle.value / (iso::pi / 2.0_p);
        if (scalar < 1.0_p) {
            return fourcc::gamma::interpolate(colors::orange_red, colors::yellow, scalar);
        } else {
            if ((1.0_p < scalar and scalar <= 1.05_p) or (1.1_p < scalar and scalar <= 1.15_p)
                or (1.2_p < scalar and scalar <= 1.25_p) or (1.3_p < scalar and scalar <= 1.35_p)) {
                return colors::black;
            } else {
                return fourcc::gamma::interpolate(colors::red, colors::orange_red, 1.0_p - scalar);
            }
        }
    }

    void radiosity(raytrace::point const& volumetric_point, precision refractive_index,
                   iso::radians const& incident_angle, iso::radians const& transmitted_angle, precision& emitted,
                   precision& reflected, precision& transmitted) const override {
        emitted = 1.0_p;
        reflected = m_reflectivity;
        transmitted = 1.0_p - m_reflectivity;
    }
};
}  // namespace outrun

// Good results at values of depth=8, subsamples=16, 1024x768, fov=23
class OutrunWorld : public world {
public:
    OutrunWorld()
        : world{raytrace::point{0, 50, 10}, raytrace::point{0, 0, 10}, "Outrun World", "world_outrun.tga"}
        , sun_center{raytrace::point{0, -3000, 200}}
        , sun_rays{raytrace::vector{0, 200, -200}, colors::white, 2 * lights::intensities::bright}
        , grid{10.0_p, outrun::neon_pink, colors::black}
        /* , schott_glass{mediums::refractive_index::glass, 0.04_p, outrun::neon_pink} */
        , floor{200.0_p}
        , sun_surface{}
        , sun{sun_center, 600.0_p}
        , s0{raytrace::point{0, 0, 5.0_p}, 5.0_p}
        , s1{raytrace::point{6, -6, 6.0_p}, 6.0_p}
        , s2{raytrace::point{-7, -7, 7.0_p}, 7.0_p}
        , e0{raytrace::point{-20, -100, 45.0_p}, 23.5_p, 5.5_p, 4.0_p}
        , e1{raytrace::point{-40, -100, 50.0_p}, 13.5_p, 7.5_p, 5.0_p}
        , pyramid0{raytrace::point{50, -100, 0}, 40.0_p}
        , pyramid1{raytrace::point{-50, -100, 0}, 40.0_p} {
        grid.mapper(std::bind(&raytrace::objects::square::map, &floor, std::placeholders::_1));
        floor.material(&grid);
        sun.material(&sun_surface);
        s0.material(&mediums::metals::gold);
        s1.material(&mediums::metals::stainless);
        s2.material(&mediums::metals::silver);
        // e0.material(&schott_glass);
        e0.material(&mediums::metals::bronze);
        e1.material(&mediums::metals::bronze);
        pyramid0.material(&grid);
        pyramid1.material(&grid);
        pyramid0.rotation(iso::degrees{0}, iso::degrees{0}, iso::degrees{90.0_p});
        pyramid1.rotation(iso::degrees{0}, iso::degrees{0}, iso::degrees{-90.0_p});
        ambient_ = outrun::neon_orange;
        ambient_.intensity(0.5_p);
    }

    ~OutrunWorld() = default;

    raytrace::color background(raytrace::ray const& world_ray) const override {
        // this creates a gradient from top to bottom
        iso::radians sky_angle = angle(R3::basis::Z, world_ray.direction());
        precision scalar = sky_angle.value / (2 * iso::pi);
        return fourcc::gamma::interpolate(outrun::neon_pink, colors::dark_slate_blue, scalar);
    }

    void add_to(scene& scene) override {
        scene.add_light(&sun_rays);
        scene.add_object(&floor);
        scene.add_object(&sun);
        scene.add_media(&mediums::earth_atmosphere);
        scene.add_object(&s0);
        scene.add_object(&s1);
        scene.add_object(&s2);
        scene.add_object(&e0);
        scene.add_object(&e1);
        scene.add_object(&pyramid0);
        scene.add_object(&pyramid1);
    }

    raytrace::animation::anchors get_anchors() const override {
        raytrace::animation::anchors anchors;
        anchors.push_back(animation::Anchor{animation::Attributes{looking_from(), looking_at(), iso::degrees{55.0_p}},
                                            animation::Attributes{looking_from(), looking_at(), iso::degrees{55.0_p}},
                                            animation::Mappers{}, iso::seconds{1.0_p}});
        return anchors;
    }

protected:
    raytrace::point sun_center;
    lights::beam sun_rays;
    raytrace::point center;
    raytrace::mediums::grid grid;
    // mediums::transparent schott_glass;
    raytrace::objects::square floor;
    outrun::Sun sun_surface;
    raytrace::objects::sphere sun;
    raytrace::objects::sphere s0;
    raytrace::objects::sphere s1;
    raytrace::objects::sphere s2;
    raytrace::objects::ellipsoid e0;
    raytrace::objects::ellipsoid e1;
    raytrace::objects::pyramid pyramid0;
    raytrace::objects::pyramid pyramid1;
};

// declare a single instance and return the reference to it
world* get_world() {
    static OutrunWorld my_world;
    return &my_world;
}
