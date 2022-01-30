#include "raytrace/medium.hpp"

namespace raytrace {

namespace functions {

color simple(const image::point& p __attribute__((unused)),
                const palette& pal __attribute__((unused))) {
    basal::exception::throw_if(pal.size() == 0, __FILE__, __LINE__, "Must have at least 1 color");
    return pal[0];
}

color checkerboard(const image::point& p, const palette& pal) {
    basal::exception::throw_unless(pal.size() == 2, __FILE__, __LINE__, "Must have only two colors in checkerboard");
    element_type h = 0.5;
    element_type u = std::fmod(p.x, 1.0);
    element_type v = std::fmod(p.y, 1.0);
    // this only really works in u > 0 && v > 0
    if (u > 0 and v > 0) {
        if ((u < h and v < h) or (u >= h and v >= h)) {
            return pal[0];
        } else {
            return pal[1];
        }
    } else if (u < 0 and v > 0) {
        if ((u > -h and v < h) or (u <= -h and v >= h)) {
            return pal[1];
        } else {
            return pal[0];
        }
    } else if (u > 0 and v < 0) {
        if ((u < h and v > -h) or (u >= h and v <= -h)) {
            return pal[1];
        } else {
            return pal[0];
        }
    } else if (u < 0 and v < 0) {
        if ((u > -h and v > -h) or (u <= -h and v <= -h)) {
            return pal[0];
        } else {
            return pal[1];
        }
    }
    // should never see this
    return colors::yellow;
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
    } else { // if (u >= 0 and v < 0 and w < 0) {
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
    } else { //if (1.5 <= upv and upv < 2.0) {
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
    element_type  r = std::sqrt(rx*rx + ry*ry);
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
    element_type  r = std::sqrt(rx*rx + ry*ry + rz*rz);
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
    const element_type l = 0.0625; // a 1/16
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
    if (l < u and u <= a and
        l < v and v <= a and
        l < w and w <= a) {
        return pal[1];
    } else {
        return pal[0];
    }
}

color pseudo_random_noise(const image::point& p, const palette& pal __attribute__((unused))) {
    noise::vector vec{{p.x, p.y}};
    // each point just gets a pixel value
    noise::vector vec_r{{ -2.199848, 7.345820}};
    noise::vector vec_g{{  9.810393, 2.928390}};
    noise::vector vec_b{{ -8.320293,-9.908293}};
    double gain = 23.83;
    double _r, r = noise::random(vec, vec_r, gain);
    double _g, g = noise::random(vec, vec_g, gain);
    double _b, b = noise::random(vec, vec_b, gain);
    color out(std::modf(r, &_r) * 255,
              std::modf(g, &_g) * 255,
              std::modf(b, &_b) * 255);
    return out;
}

} // namespace functions

} // namespace raytrace
