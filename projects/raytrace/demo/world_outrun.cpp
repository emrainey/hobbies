#include <raytrace/raytrace.hpp>
#include <vector>

#include "raytrace/color.hpp"
#include "world.hpp"

using namespace raytrace;
using namespace raytrace::objects;
using namespace raytrace::lights;
using namespace raytrace::colors;
using namespace raytrace::operators;
using namespace iso::literals;

namespace outrun {

constexpr color neon_orange(1.0_p, 0.149959790_p, 0.005605392_p);        // the ambient tone
constexpr color neon_pink(0.921581856_p, 0.000303527_p, 0.337163615_p);  // the primary grid color

/// A dimmed neon_pink for the distant ridge so the mountains recede into the sky
constexpr color dim_pink(0.414711837_p, 0.000136587_p, 0.151723626_p);

/// The layout of a single mountain peak (position, height and yaw of the base)
struct peak {
    precision x;
    precision y;
    precision h;
    precision yaw;  // rotation around the vertical axis, in degrees
};

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
            return fourcc::linear::interpolate(colors::orange, colors::yellow, scalar * scalar);
        } else {
            if ((1.0_p < scalar and scalar <= 1.05_p) or (1.1_p < scalar and scalar <= 1.15_p)
                or (1.2_p < scalar and scalar <= 1.25_p) or (1.3_p < scalar and scalar <= 1.35_p)) {
                return colors::black;
            } else {
                return fourcc::linear::interpolate(colors::red, colors::orange_red, 1.0_p - scalar);
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
        , sun_center{raytrace::point{0, -15000, 1800}}
        , sun_rays{raytrace::vector{0, 200, -500}, colors::white, lights::intensities::bright}
        , grid{10.0_p, outrun::neon_pink, colors::black}
        , mountain_grid{20.0_p, outrun::neon_pink, colors::black}
        , far_mountain_grid{30.0_p, outrun::dim_pink, colors::black}
        , floor{2200.0_p}
        , sun_surface{}
        , sun{sun_center, 2000.0_p}
        , s0{raytrace::point{0, 0, 5.0_p}, 5.0_p}
        , s1{raytrace::point{6, -6, 6.0_p}, 6.0_p}
        , s2{raytrace::point{-7, -7, 7.0_p}, 7.0_p} {
        grid.mapper(std::bind(&raytrace::objects::square::map, &floor, std::placeholders::_1));
        floor.material(&grid);
        sun.material(&sun_surface);
        s0.material(&mediums::metals::gold);
        s1.material(&mediums::metals::stainless);
        s2.material(&mediums::metals::silver);
        ambient_ = outrun::neon_orange;
        ambient_.intensity(0.5_p);
        build_mountains();
    }

    ~OutrunWorld() {
        for (auto* m : m_mountains) {
            delete m;
        }
        for (auto* m : m_far_mountains) {
            delete m;
        }
    }

    raytrace::color background(raytrace::ray const& world_ray) const override {
        // this creates a gradient from top to bottom
        iso::radians sky_angle = angle(R3::basis::Z, world_ray.direction());
        precision scalar = sky_angle.value / (2 * iso::pi);
        return fourcc::linear::interpolate(outrun::neon_pink, colors::dark_slate_blue, scalar);
    }

    void add_to(scene& scene) override {
        scene.add_light(&sun_rays);
        scene.add_object(&floor);
        scene.add_object(&sun);
        // scene.add_media(&mediums::earth_atmosphere);
        scene.add_object(&s0);
        scene.add_object(&s1);
        scene.add_object(&s2);
        for (auto* m : m_mountains) {
            scene.add_object(m);
        }
        for (auto* m : m_far_mountains) {
            scene.add_object(m);
        }
    }

    raytrace::animation::anchors get_anchors() const override {
        raytrace::animation::anchors anchors;
        anchors.push_back(animation::Anchor{animation::Attributes{looking_from(), looking_at(), iso::degrees{55.0_p}},
                                            animation::Attributes{looking_from(), looking_at(), iso::degrees{55.0_p}},
                                            animation::Mappers{}, iso::seconds{1.0_p}});
        return anchors;
    }

protected:
    /// Builds the two staggered ridges of mountains receding into the distance.
    void build_mountains() {
        // nearer ridge: lit with the bright neon grid lattice
        static outrun::peak const near_peaks[] = {
            {-170, -235, 24, 30}, {-115, -275, 48, 55}, {-55, -240, 30, 10}, {10, -305, 58, 45},
            {70, -250, 33, -20},  {125, -290, 46, 40},  {175, -245, 26, 60},
        };
        // far ridge: taller, dimmer, silhouetted against the sun
        static outrun::peak const far_peaks[] = {
            {-545, -650, 70, 20},  {-450, -790, 95, 40}, {-355, -575, 55, 0}, {-260, -875, 88, 25},
            {-165, -700, 112, 10}, {-75, -930, 102, 45}, {10, -760, 78, 5},   {105, -910, 118, 30},
            {195, -670, 62, 50},   {295, -870, 92, 15},  {390, -610, 54, 0},  {475, -740, 82, 35},
            {560, -555, 46, 20},
        };
        for (auto const& p : near_peaks) {
            auto* m = new raytrace::objects::pyramid(raytrace::point{p.x, p.y, 0.0_p}, p.h);
            m->rotation(iso::degrees{0}, iso::degrees{0}, iso::degrees{p.yaw});
            m->material(&mountain_grid);
            m_mountains.push_back(m);
        }
        for (auto const& p : far_peaks) {
            auto* m = new raytrace::objects::pyramid(raytrace::point{p.x, p.y, 0.0_p}, p.h);
            m->rotation(iso::degrees{0}, iso::degrees{0}, iso::degrees{p.yaw});
            m->material(&far_mountain_grid);
            m_far_mountains.push_back(m);
        }
    }

protected:
    raytrace::point sun_center;
    lights::beam sun_rays;
    raytrace::mediums::grid grid;
    raytrace::mediums::grid mountain_grid;
    raytrace::mediums::grid far_mountain_grid;
    raytrace::objects::square floor;
    outrun::Sun sun_surface;
    raytrace::objects::sphere sun;
    raytrace::objects::sphere s0;
    raytrace::objects::sphere s1;
    raytrace::objects::sphere s2;
    std::vector<raytrace::objects::pyramid*> m_mountains;
    std::vector<raytrace::objects::pyramid*> m_far_mountains;
};

world* get_world() {
    static OutrunWorld my_world;
    return &my_world;
}