
#include "geometry/geometry.hpp"

namespace geometry {

using namespace linalg::operators;

point::point(const size_t d) : m_data{nullptr}, dimensions{d} {
    basal::exception::throw_if(dimensions > 4, __FILE__, __LINE__, "Small dimensionality for now");
    basal::exception::throw_if(dimensions == 0, __FILE__, __LINE__, "Must be larger than zero");
    m_data = std::make_unique<precision[]>(dimensions);
    if (m_data) {
        for (size_t i = 0; i < dimensions; i++) {
            m_data[i] = 0.0;
        }
    }
}

/// Constructor from a pointer to an array
point::point(const precision a[], size_t len) noexcept(false) : point{len} {
    if (m_data) {
        for (size_t i = 0; i < dimensions; i++) {
            m_data[i] = a[i];
        }
    }
}

/// Constructor from an initialization list {{}};
point::point(const std::vector<precision>& list) noexcept(false) : point(list.size()) {
    if (m_data) {
        for (size_t i = 0; i < list.size(); i++) {
            m_data[i] = list[i];
        }
    }
}

/// Copy Constructor
point::point(const point& other) : point(other.dimensions) {
    std::copy_n(other.m_data.get(), other.dimensions, m_data.get());
}

/// Move Constructor
point::point(point&& other) : m_data(std::move(other.m_data)), dimensions(other.dimensions) {
    other.m_data = nullptr;
}

/// Destructor
point::~point() {
    m_data.reset();
}

/// Copy Assignment
point& point::operator=(const point& other) noexcept(false) {
    basal::exception::throw_unless(other.dimensions == dimensions, __FILE__, __LINE__,
                                   "Point dimensions (copy) must match point dimensions\n");
    std::copy_n(other.m_data.get(), other.dimensions, m_data.get());
    return (*this);
}

/// Move Assignment
point& point::operator=(point&& other) noexcept(false) {
    basal::exception::throw_unless(other.dimensions == dimensions, __FILE__, __LINE__,
                                   "Point dimensions (move) must match point dimensions\n");
    m_data = std::move(other.m_data);
    return (*this);
}

/// Indexer
precision& point::operator[](int i) {
    return m_data[static_cast<size_t>(i)];
}

precision point::operator[](int i) const {
    return m_data[static_cast<size_t>(i)];
}

void point::zero(void) {
    for (size_t r = 0; r < dimensions; r++) {
        m_data[r] = 0.0;
    }
}

void point::print(const char name[]) const {
    std::cout << name << " " << *this << std::endl;
}

std::ostream& operator<<(std::ostream& os, const point& p) {
    os << "point(";
    for (size_t r = 0; r < p.dimensions; r++) {
        os << p[r] << (r == (p.dimensions - 1) ? "" : ", ");
    }
    os << ")";
    return os;
}

point& point::operator*=(precision s) noexcept(false) {
    for (size_t i = 0; i < dimensions; i++) {
        m_data[i] *= s;
    }
    return (*this);
}

// NON-CLASS OPERATIONS
namespace operators {

/// Equality Operator
bool operator==(const point& a, const point& b) noexcept(false) {
    basal::exception::throw_if(a.dimensions != b.dimensions, __FILE__, __LINE__, "Points must have same dim");
    bool equal = true;
    for (size_t n = 0; n < a.dimensions; n++) {
        if (not basal::nearly_equals(a[n], b[n])) {
            equal = false;
            break;
        }
    }
    return equal;
}

point operator*(const linalg::matrix& a, const point& b) noexcept(false) {
    basal::exception::throw_unless(a.rows == b.dimensions, __FILE__, __LINE__, "");
    point c{a.rows};
    for (size_t y = 0; y < a.rows; y++) {
        for (size_t x = 0; x < a.cols; x++) {
            c[y] += a[y][x] * b[x];
        }
    }
    return point(c);
}

bool operator!=(const point& a, const point& b) noexcept(false) {
    return (not operator==(a, b));
}

bool operator==(point& a, point& b) noexcept(false) {
    basal::exception::throw_unless(a.dimensions == b.dimensions, __FILE__, __LINE__);
    bool equal = true;
    for (size_t n = 0; n < a.dimensions; n++) {
        if (a[n] != b[n]) {
            equal = false;
            break;
        }
    }
    return equal;
}

}  // namespace operators

point multiply(const point& a, precision s) noexcept(false) {
    point c{a};
    c *= s;
    return point(c);
}

point multiply(precision s, const point& a) noexcept(false) {
    point c{a};
    c *= s;
    return point(c);
}

namespace pairwise {

point multiply(const point& a, const point& b) noexcept(false) {
    basal::exception::throw_unless(a.dimensions == b.dimensions, __FILE__, __LINE__, "");
    point c{a.dimensions};
    for (size_t n = 0; n < a.dimensions; n++) {
        c[n] = a[n] * b[n];
    }
    return point(c);
}

point divide(const point& a, const point& b) noexcept(false) {
    basal::exception::throw_unless(a.dimensions == b.dimensions, __FILE__, __LINE__, "");
    point c{a.dimensions};
    for (size_t n = 0; n < a.dimensions; n++) {
        c[n] = a[n] / b[n];
    }
    return point(c);
}

}  // namespace pairwise

}  // namespace geometry
