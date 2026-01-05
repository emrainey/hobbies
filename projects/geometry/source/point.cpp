
#include "geometry/geometry.hpp"

namespace geometry {

using namespace linalg::operators;

template <size_t DIMS>
point<DIMS>::point() {
    zero();
}

/// Constructor from a pointer to an array
template <size_t DIMS>
point<DIMS>::point(precision const a[], size_t len) noexcept(false) {
    for (size_t i = 0; i < dimensions and i < len; i++) {
        m_data[i] = a[i];
    }
}

/// Constructor from an initialization list {{}};
template <size_t DIMS>
point<DIMS>::point(precision const (&list)[DIMS]) noexcept(false) {
    for (size_t i = 0; i < DIMS; i++) {
        m_data[i] = list[i];
    }
}

/// Copy Constructor
template <size_t DIMS>
point<DIMS>::point(point<DIMS> const& other) : point{} {
    memcpy(m_data, other.m_data, sizeof(precision) * dimensions);
}

/// Move Constructor
template <size_t DIMS>
point<DIMS>::point(point<DIMS>&& other) : m_data{} {
    memcpy(m_data, other.m_data, sizeof(precision) * dimensions);
}

/// Copy Assignment
template <size_t DIMS>
point<DIMS>& point<DIMS>::operator=(point<DIMS> const& other) {
    memcpy(m_data, other.m_data, sizeof(precision) * dimensions);
    return (*this);
}

/// Move Assignment
template <size_t DIMS>
point<DIMS>& point<DIMS>::operator=(point<DIMS>&& other) {
    std::copy_n(other.m_data, other.dimensions, m_data);
    return (*this);
}

/// Indexer
template <size_t DIMS>
precision& point<DIMS>::operator[](size_t i) {
    return m_data[i];
}

template <size_t DIMS>
precision point<DIMS>::operator[](size_t i) const {
    return m_data[i];
}

template <size_t DIMS>
void point<DIMS>::zero(void) {
    for (size_t r = 0; r < dimensions; r++) {
        m_data[r] = 0.0_p;
    }
}

template <size_t DIMS>
void point<DIMS>::print(std::ostream& os, char const name[]) const {
    os << name << " " << *this << std::endl;
}

template <size_t DIMS>
std::ostream& operator<<(std::ostream& os, point<DIMS> const& p) {
    os << "point(";
    for (size_t r = 0; r < p.dimensions; r++) {
        os << p[r] << (r == (p.dimensions - 1) ? "" : ", ");
    }
    os << ")";
    return os;
}

template <size_t DIMS>
point<DIMS>& point<DIMS>::operator*=(precision s) {
    for (size_t i = 0; i < dimensions; i++) {
        m_data[i] *= s;
    }
    return (*this);
}

// NON-CLASS OPERATIONS
namespace operators {

/// Equality Operator
template <size_t DIMS>
bool operator==(point<DIMS> const& a, point<DIMS> const& b) {
    bool equal = true;
    for (size_t n = 0; n < a.dimensions; n++) {
        if (not basal::nearly_equals(a[n], b[n])) {
            equal = false;
            break;
        }
    }
    return equal;
}

template <size_t DIMS>
bool operator!=(point<DIMS> const& a, point<DIMS> const& b) {
    return not operator==(a, b);
}

template <size_t DIMS>
point<DIMS> operator*(linalg::matrix const& a, point<DIMS> const& b) {
    // basal::exception::throw_unless(a.rows == b.dimensions, __FILE__, __LINE__, "");
    point<DIMS> c;
    for (size_t y = 0; y < a.rows; y++) {
        for (size_t x = 0; x < a.cols; x++) {
            c[y] += a[y][x] * b[x];
        }
    }
    return c;
}

}  // namespace operators

template <size_t DIMS>
point<DIMS> multiply(point<DIMS> const& a, precision s) noexcept(false) {
    point<DIMS> c{a};
    c *= s;
    return c;
}

template <size_t DIMS>
point<DIMS> multiply(precision s, point<DIMS> const& a) noexcept(false) {
    point<DIMS> c{a};
    c *= s;
    return c;
}

namespace pairwise {

template <size_t DIMS>
point<DIMS> multiply(point<DIMS> const& a, point<DIMS> const& b) noexcept(false) {
    basal::exception::throw_unless(a.dimensions == b.dimensions, __FILE__, __LINE__, "");
    point<DIMS> c;
    for (size_t n = 0; n < a.dimensions; n++) {
        c[n] = a[n] * b[n];
    }
    return c;
}

template <size_t DIMS>
point<DIMS> divide(point<DIMS> const& a, point<DIMS> const& b) noexcept(false) {
    basal::exception::throw_unless(a.dimensions == b.dimensions, __FILE__, __LINE__, "");
    point<DIMS> c;
    for (size_t n = 0; n < a.dimensions; n++) {
        c[n] = a[n] / b[n];
    }
    return c;
}

}  // namespace pairwise

// Explicit Instantiations for 2D
template class point<2ul>;
template bool operators::operator== <2ul>(point<2ul> const&, point<2ul> const&);
template bool operators::operator!= <2ul>(point<2ul> const&, point<2ul> const&);
template point<2ul> operators::operator* <2ul>(linalg::matrix const&, point<2ul> const&);
template point<2ul> multiply<2ul>(point<2ul> const&, double);
template point<2ul> multiply<2ul>(double, point<2ul> const&);
template point<2ul> pairwise::divide<2ul>(point<2ul> const&, point<2ul> const&);
template point<2ul> pairwise::multiply<2ul>(point<2ul> const&, point<2ul> const&);
template std::ostream& operator<< <2ul>(std::ostream&, point<2ul> const&);

// Explicit Instantiations for 3D
template class point<3ul>;
template bool operators::operator== <3ul>(point<3ul> const&, point<3ul> const&);
template bool operators::operator!= <3ul>(point<3ul> const&, point<3ul> const&);
template point<3ul> operators::operator* <3ul>(linalg::matrix const&, point<3ul> const&);
template point<3ul> multiply<3ul>(point<3ul> const&, double);
template point<3ul> multiply<3ul>(double, point<3ul> const&);
template point<3ul> pairwise::divide<3ul>(point<3ul> const&, point<3ul> const&);
template point<3ul> pairwise::multiply<3ul>(point<3ul> const&, point<3ul> const&);
template std::ostream& operator<< <3ul>(std::ostream&, point<3ul> const&);

// Explicit Instantiations for 4D
template class point<4ul>;
template bool operators::operator== <4ul>(point<4ul> const&, point<4ul> const&);
template bool operators::operator!= <4ul>(point<4ul> const&, point<4ul> const&);
template point<4ul> operators::operator* <4ul>(linalg::matrix const&, point<4ul> const&);
template point<4ul> multiply<4ul>(point<4ul> const&, double);
template point<4ul> multiply<4ul>(double, point<4ul> const&);
template point<4ul> pairwise::divide<4ul>(point<4ul> const&, point<4ul> const&);
template point<4ul> pairwise::multiply<4ul>(point<4ul> const&, point<4ul> const&);
template std::ostream& operator<< <4ul>(std::ostream&, point<4ul> const&);

}  // namespace geometry
