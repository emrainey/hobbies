#pragma once

#include <basal/printable.hpp>

#include "geometry/line.hpp"
#include "geometry/point.hpp"
#include "geometry/vector.hpp"

namespace geometry {

using namespace geometry::operators;

/** This ray is assumed to be a 3D ray */
template <size_t DIMS>
class ray_ : public basal::printable {
    static_assert(2 <= DIMS and DIMS <= 4, "Must be either 2 to 4 dimensional");

public:
    using point = point_<DIMS>;
    using vector = vector_<precision, DIMS>;

    /** Remember how many base dimensions this ray has */
    constexpr static size_t dimensions = DIMS;

    /** Default constructor */
    explicit ray_() : m_location{}, m_direction{} {
    }

    /** Parameter Costructor */
    explicit ray_(const point& s, const vector& v) : m_location{s}, m_direction{v} {
    }

    /** Destructor */
    ~ray_() = default;

    /** Default Copy */
    ray_(const ray_<DIMS>& other) = default;

    /** Default Move */
    ray_(ray_<DIMS>&& other) = default;

    /** Default Assign */
    ray_<DIMS>& operator=(const ray_<DIMS>& other) = default;

    /** Default Move Assign */
    ray_<DIMS>& operator=(ray_<DIMS>&& other) = default;

    /** Returns a readonly reference to the location */
    const point& location() const {
        return m_location;
    }

    /** Returns a readonly refernce to the direction */
    const vector& direction() const {
        return m_direction;
    }

    point distance_along(precision t) const {
        return m_location + (t * m_direction);
    }

    point closest(const R3::point& p) const {
        R3::vector side = p - location();
        precision t = dot(side, direction()) / direction().quadrance();
        return distance_along(t);
    }

    void print(const char str[]) const override {
        std::cout << str << " " << *this << std::endl;
    }

protected:
    point m_location;
    vector m_direction;
};

template <size_t DIMS>
std::ostream& operator<<(std::ostream& os, const ray_<DIMS>& ray) {
    os << "ray(" << ray.location() << "," << ray.direction() << ")";
    return os;
}

/** Cross two rays to make a right angle ray to the first two.
 * @warning This only is possible for rays which share the same location.
 */
template <size_t DIMS>
ray_<DIMS> cross(const ray_<DIMS>& a, const ray_<DIMS>& b) {
    static_assert(DIMS == 3, "Can only cross a 3D ray");
    basal::exception::throw_unless(a.location() == b.location(), __FILE__, __LINE__, "Must be at the same location");
    return ray_<DIMS>(a.location(), R3::cross(a.direction(), b.direction()));
}

template <size_t DIMS>
bool equality(const ray_<DIMS>& a, const ray_<DIMS>& b) {
    return a.location() == b.location() and b.direction() == b.direction();
}

template <size_t DIMS>
bool inequality(const ray_<DIMS>& a, const ray_<DIMS>& b) {
    return not equality(a, b);
}

/** Multiplies the direction of the ray to "rotate" it in space. */
template <size_t DIMS>
ray_<DIMS> multiply(const linalg::matrix& m, const ray_<DIMS>& r) {
    basal::exception::throw_unless(m.rows == r.dimensions, __FILE__, __LINE__, "Must be same dimensions");
    vector_<precision, DIMS> v = multiply(m, r.direction());
    return ray_<DIMS>(r.location(), v);
}

/** Adds the vector to the point, does change the direction of the ray */
template <size_t DIMS>
ray_<DIMS> addition(const ray_<DIMS>& r, const vector_<precision, DIMS>& v) {
    return ray_<DIMS>(r.location() + v, r.direction());
}

namespace operators {
/** Equality Operator */
template <size_t DIMS>
inline bool operator==(const ray_<DIMS>& a, const ray_<DIMS>& b) {
    return equality(a, b);
}

/** Inequality operator */
template <size_t DIMS>
inline bool operator!=(const ray_<DIMS>& a, const ray_<DIMS>& b) {
    return inequality(a, b);
}

/** Multiply Operator */
template <size_t DIMS>
inline ray_<DIMS> operator*(const linalg::matrix& m, const ray_<DIMS>& r) {
    return multiply(m, r);
}

/** Addition Operator. Add the vector to the point, does change the direction of the ray */
template <size_t DIMS>
inline ray_<DIMS> operator+(const ray_<DIMS>& r, const vector_<precision, DIMS>& v) {
    return addition(r, v);
}
}  // namespace operators

namespace R2 {
using ray = ray_<dimensions>;
}

namespace R3 {
using ray = ray_<dimensions>;

/** Returns a line which is co-linear to the ray */
inline R3::line as_line(const ray& r) {
    return R3::line(r.direction(), r.location());
}
}  // namespace R3

}  // namespace geometry
