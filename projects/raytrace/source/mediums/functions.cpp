#include "raytrace/mediums/medium.hpp"

namespace raytrace {

namespace functions {

color simple(image::point const& p __attribute__((unused)), palette const& pal __attribute__((unused))) {
    basal::exception::throw_unless(pal.size() >= 1, __FILE__, __LINE__, "Must have at least 1 color");
    return pal[0];
}

color checkerboard(image::point const& p, palette const& pal) {
    basal::exception::throw_unless(pal.size() >= 8, __FILE__, __LINE__, "Must have at least 8 colors in checkerboard");
    precision h = 0.5_p;
    precision u = std::fmod(p.x, 1.0_p);  // values between -1.0_p and 1.0_p exclusive
    precision v = std::fmod(p.y, 1.0_p);  // values between -1.0_p and 1.0_p exclusive

    const bool u_positive = basal::is_greater_than_or_equal_to_zero(u);
    const bool v_positive = basal::is_greater_than_or_equal_to_zero(v);

    // this only really works in u > 0 && v > 0
    if (u_positive and v_positive) {  // quad 1
        if ((u < h and v < h) or (u >= h and v >= h)) {
            return pal[0];
        } else {
            return pal[1];
        }
    } else if ((not u_positive) and v_positive) {  // quad 2
        if ((u > -h and v < h) or (u <= -h and v >= h)) {
            return pal[3];
        } else {
            return pal[2];
        }
    } else if (u_positive and (not v_positive)) {  // quad 4
        if ((u < h and v > -h) or (u >= h and v <= -h)) {
            return pal[7];
        } else {
            return pal[6];
        }
    } else if ((not u_positive) and (not v_positive)) {  // quad 3
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

color checkerboard(raytrace::point const& p, palette const& pal) {
    // palette could have up to 8 colors, don't worry about it here
    basal::exception::throw_unless(pal.size() >= 2, __FILE__, __LINE__,
                                   "Must have at least two colors in checkerboard");
    static constexpr precision h = 0.5_p;
    precision u = std::fmod(p.x, 1.0_p);  // values between -1.0_p and 1.0_p exclusive
    precision v = std::fmod(p.y, 1.0_p);  // values between -1.0_p and 1.0_p exclusive
    precision w = std::fmod(p.z, 1.0_p);  // values between -1.0_p and 1.0_p exclusive

    const bool u_positive = basal::is_greater_than_or_equal_to_zero(u);
    const bool v_positive = basal::is_greater_than_or_equal_to_zero(v);
    const bool w_positive = basal::is_greater_than_or_equal_to_zero(w);

    // there are 8 cases of p/ne (if you just do std::abs, you get a weird pattern)
    if (u_positive and v_positive and w_positive) {
        if ((u < h and v < h) or (u >= h and v >= h)) {
            return (w < h ? pal[0] : pal[1]);
        } else {
            return (w < h ? pal[1] : pal[0]);
        }
    } else if ((not u_positive) and v_positive and w_positive) {
        if ((u < -h and v < h) or (u >= -h and v >= h)) {
            return (w < h ? pal[0] : pal[1]);
        } else {
            return (w < h ? pal[1] : pal[0]);
        }
    } else if ((not u_positive) and (not v_positive) and w_positive) {
        if ((u < -h and v < -h) or (u >= -h and v >= -h)) {
            return (w < h ? pal[0] : pal[1]);
        } else {
            return (w < h ? pal[1] : pal[0]);
        }
    } else if (u_positive and (not v_positive) and w_positive) {
        if ((u < h and v < -h) or (u >= h and v >= -h)) {
            return (w < h ? pal[0] : pal[1]);
        } else {
            return (w < h ? pal[1] : pal[0]);
        }
    } else if (u_positive and v_positive and (not w_positive)) {
        if ((u < h and v < h) or (u >= h and v >= h)) {
            return (w < -h ? pal[0] : pal[1]);
        } else {
            return (w < -h ? pal[1] : pal[0]);
        }
    } else if ((not u_positive) and v_positive and (not w_positive)) {
        if ((u < -h and v < h) or (u >= -h and v >= h)) {
            return (w < -h ? pal[0] : pal[1]);
        } else {
            return (w < -h ? pal[1] : pal[0]);
        }
    } else if ((not u_positive) and (not v_positive) and (not w_positive)) {
        if ((u < -h and v < -h) or (u >= -h and v >= -h)) {
            return (w < -h ? pal[0] : pal[1]);
        } else {
            return (w < -h ? pal[1] : pal[0]);
        }
    } else {  // if (u_positive and (not v_positive) and (not w_positive)) {
        if ((u < h and v < -h) or (u >= h and v >= -h)) {
            return (w < -h ? pal[0] : pal[1]);
        } else {
            return (w < -h ? pal[1] : pal[0]);
        }
    }
}

color diagonal(raytrace::point const& p, palette const& pal) {
    basal::exception::throw_unless(pal.size() >= 2, __FILE__, __LINE__,
                                   "Must have at least two colors in checkerboard");
    precision u = std::fmod(p.x, 1.0_p);
    precision v = std::fmod(p.y, 1.0_p);
    precision w = std::fmod(p.z, 1.0_p);
    precision upvw = u + v + w;
    if ((0.0_p <= upvw and upvw < 0.5_p)) {
        return pal[0];
    } else if (0.5_p <= upvw and upvw < 1.0_p) {
        return pal[1];
    } else if (1.0_p <= upvw and upvw < 1.5_p) {
        return pal[0];
    } else {  // if (1.5_p <= upvw and upvw < 2.0_p) {
        return pal[1];
    }
}

color diagonal(image::point const& p, palette const& pal) {
    basal::exception::throw_unless(pal.size() >= 2, __FILE__, __LINE__,
                                   "Must have at least two colors in checkerboard");
    precision u = std::fmod(p.x, 1.0_p);
    precision v = std::fmod(p.y, 1.0_p);
    precision upv = u + v;
    if (-2.0_p <= upv and upv < -1.5_p) {
        return pal[0];
    } else if (-1.5_p <= upv and upv < -1.0_p) {
        return pal[1];
    } else if (-1.0_p <= upv and upv < -0.5_p) {
        return pal[0];
    } else if (-0.5_p <= upv and upv < 0.0_p) {
        return pal[1];
    } else if ((0.0_p <= upv and upv < 0.5_p)) {
        return pal[0];
    } else if (0.5_p <= upv and upv < 1.0_p) {
        return pal[1];
    } else if (1.0_p <= upv and upv < 1.5_p) {
        return pal[0];
    } else {  // if (1.5_p <= upv and upv < 2.0_p) {
        return pal[1];
    }
}

color dots(image::point const& p, palette const& pal) {
    basal::exception::throw_unless(pal.size() >= 2, __FILE__, __LINE__,
                                   "Must have at least two colors in checkerboard");
    precision u = std::abs(std::fmod(p.x, 1.0_p));
    precision v = std::abs(std::fmod(p.y, 1.0_p));
    precision h = 0.5_p;
    precision rx = std::abs(h - u);
    precision ry = std::abs(h - v);
    precision r = std::sqrt(rx * rx + ry * ry);
    if (r < 0.3_p) {
        return pal[1];
    } else {
        return pal[0];
    }
}

color dots(raytrace::point const& p, palette const& pal) {
    basal::exception::throw_unless(pal.size() >= 2, __FILE__, __LINE__,
                                   "Must have at least two colors in checkerboard");
    precision u = std::abs(std::fmod(p.x, 1.0_p));
    precision v = std::abs(std::fmod(p.y, 1.0_p));
    precision w = std::abs(std::fmod(p.z, 1.0_p));
    precision h = 0.5_p;
    precision rx = std::abs(h - u);
    precision ry = std::abs(h - v);
    precision rz = std::abs(h - w);
    precision r = std::sqrt(rx * rx + ry * ry + rz * rz);
    if (r < 0.3_p) {
        return pal[1];
    } else {
        return pal[0];
    }
}

color grid(image::point const& p, palette const& pal) {
    basal::exception::throw_unless(pal.size() >= 2, __FILE__, __LINE__,
                                   "Must have at least two colors in checkerboard");
    precision u = std::abs(std::fmod(p.x, 1.0_p));
    precision v = std::abs(std::fmod(p.y, 1.0_p));
    precision const l = 1.0_p / 32.0_p;  // line width
    precision const a = 1.0_p - l;
    if (l < u and u <= a and l < v and v <= a) {
        return pal[1];
    } else {
        return pal[0];
    }
}

color grid(raytrace::point const& p, palette const& pal) {
    basal::exception::throw_unless(pal.size() >= 2, __FILE__, __LINE__,
                                   "Must have at least two colors in checkerboard");
    precision u = std::abs(std::fmod(p.x, 1.0_p));
    precision v = std::abs(std::fmod(p.y, 1.0_p));
    precision w = std::abs(std::fmod(p.z, 1.0_p));
    precision const l = 1.0_p / 32.0_p;  // line width
    precision const a = 1.0_p - l;
    if (l < u and u <= a and l < v and v <= a and l < w and w <= a) {
        return pal[1];
    } else {
        return pal[0];
    }
}

color pseudo_random_noise(image::point const& p, palette const& pal __attribute__((unused))) {
    noise::vector vec{{p.x, p.y}};
    // if you normalize the vector, you get a circular pattern
    // vec = (vec == geometry::R2::null) ? geometry::R2::null : vec.normalized();
    precision _r,
        r = noise::random(vec, tuning::pseudo_random_noise_params.vec_r, tuning::pseudo_random_noise_params.gain);
    precision _g,
        g = noise::random(vec, tuning::pseudo_random_noise_params.vec_g, tuning::pseudo_random_noise_params.gain);
    precision _b,
        b = noise::random(vec, tuning::pseudo_random_noise_params.vec_b, tuning::pseudo_random_noise_params.gain);
    precision px = std::modf(r, &_r);
    precision py = std::modf(g, &_g);
    precision pz = std::modf(b, &_b);
    return color{px, py, pz};
}

color happy_face(raytrace::point const& p, palette const& pal) {
    basal::exception::throw_unless(pal.size() >= 2, __FILE__, __LINE__,
                                   "Must have at least two colors in palette for happy-face");
    precision u = std::abs(std::fmod(p.x, 1.0_p));
    precision v = std::abs(std::fmod(p.y, 1.0_p));
    // precision w = std::abs(std::fmod(p.z, 1.0_p));
    image::point uv{u, v};
    precision const eye_radius = 1.0_p / 9.0_p;
    precision const mouth_radius = 2.0_p / 9.0_p;
    image::point le{3.0_p / 9.0_p, 3.0_p / 9.0_p};
    image::point re{6.0_p / 9.0_p, 3.0_p / 9.0_p};
    image::point mc{0.5_p, 0.5_p};
    precision dle = (uv - le).magnitude();
    precision dre = (uv - re).magnitude();
    precision dmt = (uv - mc).magnitude();
    if (dle < eye_radius) {
        return pal[0];
    } else if (dre < eye_radius) {
        return pal[0];
    } else if (dmt < mouth_radius) {
        image::vector mv{0.0_p, -1.0_p};
        precision md = dot(mv, (uv - mc));
        if (md < 0.0_p) {
            return pal[0];
        }
    }
    return pal[1];
}

color happy_face(image::point const& p, palette const& pal) {
    basal::exception::throw_unless(pal.size() >= 2, __FILE__, __LINE__,
                                   "Must have at least two colors in palette for happy-face");
    precision u = std::abs(std::fmod(p.x, 1.0_p));
    precision v = std::abs(std::fmod(p.y, 1.0_p));
    image::point uv{u, v};
    precision const eye_radius = 1.0_p / 9.0_p;
    precision const mouth_radius = 2.0_p / 9.0_p;
    image::point le{3.0_p / 9.0_p, 3.0_p / 9.0_p};
    image::point re{6.0_p / 9.0_p, 3.0_p / 9.0_p};
    image::point mc{0.5_p, 0.5_p};
    precision dle = (uv - le).magnitude();
    precision dre = (uv - re).magnitude();
    precision dmt = (uv - mc).magnitude();
    if (dle < eye_radius) {
        return pal[0];
    } else if (dre < eye_radius) {
        return pal[0];
    } else if (dmt < mouth_radius) {
        image::vector mv{0.0_p, -1.0_p};
        precision md = dot(mv, (uv - mc));
        if (md < 0.0_p) {
            return pal[0];
        }
    }
    return pal[1];
}

color contours(image::point const& p, palette const& pal) {
    basal::exception::throw_unless(pal.size() >= 8, __FILE__, __LINE__,
                                   "Must have at least two colors in palette for happy-face");
    precision u = std::abs(std::fmod(p.x, 1.0_p));
    precision v = std::abs(std::fmod(p.y, 1.0_p));
    image::point const uv{u, v};
    image::point const a{0.25_p, 0.5_p};
    image::point const b{0.75_p, 0.5_p};
    precision const delta = 0.125_p;
    precision a1 = (p - a).magnitude();
    precision b1 = (p - b).magnitude();
    precision r1 = a1 / b1;

    if ((0.875_p - delta) <= r1 and r1 < (0.875_p + delta)) {
        return pal[7];
    } else if ((0.750_p - delta) <= r1 and r1 < (0.750_p + delta)) {
        return pal[6];
    } else if ((0.625_p - delta) <= r1 and r1 < (0.625_p + delta)) {
        return pal[5];
    } else if ((0.5_p - delta) <= r1 and r1 < (0.5_p + delta)) {
        return pal[4];
    } else if ((0.375_p - delta) <= r1 and r1 < (0.375_p + delta)) {
        return pal[3];
    } else if ((0.25_p - delta) <= r1 and r1 < (0.25_p + delta)) {
        return pal[2];
    } else if ((0.125_p - delta) <= r1 and r1 < (0.125_p + delta)) {
        return pal[1];
    } else {
        return pal[0];
    }
}

}  // namespace functions

namespace tuning {

prn_parameters::prn_parameters() : _initialized{false} {
    initialize();
}

void prn_parameters::initialize(bool again) {
    if (not _initialized or (_initialized and again)) {
        gain = 1.0_p;
        radius = 1.0_p;
        theta_r = iso::radians(iso::tau * 0.5_p);
        theta_g = iso::radians(iso::tau * 0.2_p);
        theta_b = iso::radians(iso::tau * 0.8_p);
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

    if (debug::generic) {
        std::cout << "Gain: " << gain << " Radius: " << radius << std::endl;
        std::cout << "Theta R/pi: " << theta_r.value / iso::pi << " Vec R: " << vec_r << std::endl;
        std::cout << "Theta G/pi: " << theta_g.value / iso::pi << " Vec G: " << vec_g << std::endl;
        std::cout << "Theta B/pi: " << theta_b.value / iso::pi << " Vec B: " << vec_b << std::endl;
    }
}

// The global instance of the parameters used by the pseudo_random_noise
prn_parameters pseudo_random_noise_params;

}  // namespace tuning

}  // namespace raytrace
