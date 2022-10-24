#include "raytrace/mediums/medium.hpp"

namespace raytrace {

namespace functions {

color simple(const image::point& p __attribute__((unused)), const palette& pal __attribute__((unused))) {
    basal::exception::throw_if(pal.size() == 0, __FILE__, __LINE__, "Must have at least 1 color");
    return pal[0];
}

color checkerboard(const image::point& p, const palette& pal) {
    basal::exception::throw_unless(pal.size() == 8, __FILE__, __LINE__, "Must have all 8 colors in checkerboard");
    element_type h = 0.5;
    element_type u = std::fmod(p.x, 1.0);  // values betwee n -1.0 and 1.0 exclusive
    element_type v = std::fmod(p.y, 1.0);  // values betwee n -1.0 and 1.0 exclusive
    // this only really works in u > 0 && v > 0
    if (u >= 0 and v >= 0) {  // quad 1
        if ((u < h and v < h) or (u >= h and v >= h)) {
            return pal[0];
        } else {
            return pal[1];
        }
    } else if (u < 0 and v >= 0) {  // quad 2
        if ((u > -h and v < h) or (u <= -h and v >= h)) {
            return pal[3];
        } else {
            return pal[2];
        }
    } else if (u >= 0 and v < 0) {  // quad 4
        if ((u < h and v > -h) or (u >= h and v <= -h)) {
            return pal[7];
        } else {
            return pal[6];
        }
    } else if (u < 0 and v < 0) {  // quad 3
        if ((u > -h and v > -h) or (u <= -h and v <= -h)) {
            return pal[4];
        } else {
            return pal[5];
        }
    }
    // should never see this
    std::cerr << "Bad coordinates " << p << " u:" << u << " v:" << v << std::endl;
    return colors::cyan;
}

color checkerboard(const raytrace::point& p, const palette& pal) {
    basal::exception::throw_unless(pal.size() == 2, __FILE__, __LINE__, "Must have only two colors in checkerboard");
    static constexpr element_type h = 0.5;
    element_type u = std::fmod(p.x, 1.0);
    element_type v = std::fmod(p.y, 1.0);
    element_type w = std::fmod(p.z, 1.0);

    // there are 8 cases of p/ne (if you just do std::abs, you get a weird pattern)
    if (u >= 0 and v >= 0 and w >= 0) {
        if ((u < h and v < h) or (u >= h and v >= h)) {
            return (w < h ? pal[0] : pal[1]);
        } else {
            return (w < h ? pal[1] : pal[0]);
        }
    } else if (u < 0 and v >= 0 and w >= 0) {
        if ((u < -h and v < h) or (u >= -h and v >= h)) {
            return (w < h ? pal[0] : pal[1]);
        } else {
            return (w < h ? pal[1] : pal[0]);
        }
    } else if (u < 0 and v < 0 and w >= 0) {
        if ((u < -h and v < -h) or (u >= -h and v >= -h)) {
            return (w < h ? pal[0] : pal[1]);
        } else {
            return (w < h ? pal[1] : pal[0]);
        }
    } else if (u >= 0 and v < 0 and w >= 0) {
        if ((u < h and v < -h) or (u >= h and v >= -h)) {
            return (w < h ? pal[0] : pal[1]);
        } else {
            return (w < h ? pal[1] : pal[0]);
        }
    } else if (u >= 0 and v >= 0 and w < 0) {
        if ((u < h and v < h) or (u >= h and v >= h)) {
            return (w < -h ? pal[0] : pal[1]);
        } else {
            return (w < -h ? pal[1] : pal[0]);
        }
    } else if (u < 0 and v >= 0 and w < 0) {
        if ((u < -h and v < h) or (u >= -h and v >= h)) {
            return (w < -h ? pal[0] : pal[1]);
        } else {
            return (w < -h ? pal[1] : pal[0]);
        }
    } else if (u < 0 and v < 0 and w < 0) {
        if ((u < -h and v < -h) or (u >= -h and v >= -h)) {
            return (w < -h ? pal[0] : pal[1]);
        } else {
            return (w < -h ? pal[1] : pal[0]);
        }
    } else {  // if (u >= 0 and v < 0 and w < 0) {
        if ((u < h and v < -h) or (u >= h and v >= -h)) {
            return (w < -h ? pal[0] : pal[1]);
        } else {
            return (w < -h ? pal[1] : pal[0]);
        }
    }
}

color diagonal(const image::point& p, const palette& pal) {
    basal::exception::throw_unless(pal.size() == 2, __FILE__, __LINE__, "Must have only two colors in checkerboard");
    element_type u = std::fmod(p.x, 1.0);
    element_type v = std::fmod(p.y, 1.0);
    element_type upv = u + v;
    if ((0.0 <= upv and upv < 0.5)) {
        return pal[0];
    } else if (0.5 <= upv and upv < 1.0) {
        return pal[1];
    } else if (1.0 <= upv and upv < 1.5) {
        return pal[0];
    } else {  // if (1.5 <= upv and upv < 2.0) {
        return pal[1];
    }
}

color dots(const image::point& p, const palette& pal) {
    basal::exception::throw_unless(pal.size() == 2, __FILE__, __LINE__, "Must have only two colors in checkerboard");
    element_type u = std::abs(std::fmod(p.x, 1.0));
    element_type v = std::abs(std::fmod(p.y, 1.0));
    element_type h = 0.5;
    element_type rx = std::abs(h - u);
    element_type ry = std::abs(h - v);
    element_type r = std::sqrt(rx * rx + ry * ry);
    if (r < 0.3) {
        return pal[1];
    } else {
        return pal[0];
    }
}

color dots(const raytrace::point& p, const palette& pal) {
    basal::exception::throw_unless(pal.size() == 2, __FILE__, __LINE__, "Must have only two colors in checkerboard");
    element_type u = std::abs(std::fmod(p.x, 1.0));
    element_type v = std::abs(std::fmod(p.y, 1.0));
    element_type w = std::abs(std::fmod(p.z, 1.0));
    element_type h = 0.5;
    element_type rx = std::abs(h - u);
    element_type ry = std::abs(h - v);
    element_type rz = std::abs(h - w);
    element_type r = std::sqrt(rx * rx + ry * ry + rz * rz);
    if (r < 0.3) {
        return pal[1];
    } else {
        return pal[0];
    }
}

color grid(const image::point& p, const palette& pal) {
    basal::exception::throw_unless(pal.size() == 2, __FILE__, __LINE__, "Must have only two colors in checkerboard");
    element_type u = std::abs(std::fmod(p.x, 1.0));
    element_type v = std::abs(std::fmod(p.y, 1.0));
    const element_type l = 0.0625;  // a 1/16
    const element_type a = 1.0 - l;
    if (l < u and u <= a and l < v and v <= a) {
        return pal[1];
    } else {
        return pal[0];
    }
}

color grid(const raytrace::point& p, const palette& pal) {
    basal::exception::throw_unless(pal.size() == 2, __FILE__, __LINE__, "Must have only two colors in checkerboard");
    element_type u = std::abs(std::fmod(p.x, 1.0));
    element_type v = std::abs(std::fmod(p.y, 1.0));
    element_type w = std::abs(std::fmod(p.z, 1.0));
    const element_type l = 0.0625;
    const element_type a = 1.0 - l;
    if (l < u and u <= a and l < v and v <= a and l < w and w <= a) {
        return pal[1];
    } else {
        return pal[0];
    }
}

color pseudo_random_noise(const image::point& p, const palette& pal __attribute__((unused))) {
    noise::vector vec{{p.x, p.y}};
    vec = (vec == geometry::R2::null) ? geometry::R2::null : vec.normalized();
    double _r,
        r = noise::random(vec, tuning::pseudo_random_noise_params.vec_r, tuning::pseudo_random_noise_params.gain);
    double _g,
        g = noise::random(vec, tuning::pseudo_random_noise_params.vec_g, tuning::pseudo_random_noise_params.gain);
    double _b,
        b = noise::random(vec, tuning::pseudo_random_noise_params.vec_b, tuning::pseudo_random_noise_params.gain);
    color out(std::modf(r, &_r) * 255, std::modf(g, &_g) * 255, std::modf(b, &_b) * 255);
    return out;
}

}  // namespace functions

namespace tuning {

prn_parameters::prn_parameters() : _initialized{false} {
    initialize();
}

void prn_parameters::initialize(bool again) {
    if (not _initialized or (_initialized and again)) {
        gain = 1.0;
        radius = 1.0;
        theta_r = iso::radians(iso::pi * 0.5);
        theta_g = iso::radians(iso::pi * 0.2);
        theta_b = iso::radians(iso::pi * 0.8);
        _initialized = true;
    }
}

void prn_parameters::update() {
    vec_r[0] = radius * std::cos(theta_r.value);
    vec_r[1] = radius * std::sin(theta_r.value);
    vec_g[0] = radius * std::cos(theta_g.value);
    vec_g[1] = radius * std::sin(theta_g.value);
    vec_b[0] = radius * std::cos(theta_b.value);
    vec_b[1] = radius * std::sin(theta_b.value);

    std::cout << "Gain: " << gain << " Radius: " << radius << std::endl;
    std::cout << "Theta R/pi: " << theta_r.value / iso::pi << " Vec R: " << vec_r << std::endl;
    std::cout << "Theta G/pi: " << theta_g.value / iso::pi << " Vec G: " << vec_g << std::endl;
    std::cout << "Theta B/pi: " << theta_b.value / iso::pi << " Vec B: " << vec_b << std::endl;
}

// The global instance of the parameters used by the pseudo_random_noise
prn_parameters pseudo_random_noise_params;

}  // namespace tuning

}  // namespace raytrace
