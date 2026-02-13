
#include "geometry/geometry.hpp"

namespace geometry {

using namespace linalg::operators;

template <size_t DIMS>
point_<DIMS>::point_() {
    zero();
}

/// Constructor from a pointer to an array
template <size_t DIMS>
point_<DIMS>::point_(precision const a[], size_t len) noexcept(false) {
    for (size_t i = 0; i < dimensions and i < len; i++) {
        data_[i] = a[i];
    }
}

/// Constructor from an initialization list {{}};
template <size_t DIMS>
point_<DIMS>::point_(precision const (&list)[DIMS]) noexcept(false) {
    for (size_t i = 0; i < DIMS; i++) {
        data_[i] = list[i];
    }
}

/// Copy Constructor
template <size_t DIMS>
point_<DIMS>::point_(point_<DIMS> const& other) : point_{} {
    memcpy(data_, other.data_, sizeof(precision) * dimensions);
}

/// Move Constructor
template <size_t DIMS>
point_<DIMS>::point_(point_<DIMS>&& other) : data_{} {
    memcpy(data_, other.data_, sizeof(precision) * dimensions);
}

/// Copy Assignment
template <size_t DIMS>
point_<DIMS>& point_<DIMS>::operator=(point_<DIMS> const& other) {
    memcpy(data_, other.data_, sizeof(precision) * dimensions);
    return (*this);
}

/// Move Assignment
template <size_t DIMS>
point_<DIMS>& point_<DIMS>::operator=(point_<DIMS>&& other) {
    std::copy_n(other.data_, other.dimensions, data_);
    return (*this);
}

/// Constructor with 2 parameters (enabled for DIMS == 2)
template <size_t DIMS>
template <size_t D, typename>
point_<DIMS>::point_(precision a, precision b) : point_{} {
    data_[0] = a;
    data_[1] = b;
}

/// Constructor with 3 parameters (enabled for DIMS == 3)
template <size_t DIMS>
template <size_t D, typename>
point_<DIMS>::point_(precision a, precision b, precision c) : point_{} {
    data_[0] = a;
    data_[1] = b;
    data_[2] = c;
}

/// Constructor with 4 parameters (enabled for DIMS == 4)
template <size_t DIMS>
template <size_t D, typename>
point_<DIMS>::point_(precision a, precision b, precision c, precision d) : point_{} {
    data_[0] = a;
    data_[1] = b;
    data_[2] = c;
    data_[3] = d;
}

/// Homogenizing constructor from 2D to 3D with z=1
template <size_t DIMS>
template <size_t D, typename>
point_<DIMS>::point_(point_<2> const& p) : point_{} {
    data_[0] = p.x();
    data_[1] = p.y();
    data_[2] = 1.0_p;
}

/// Homogenizing constructor from 3D to 4D with w=1
template <size_t DIMS>
template <size_t D, typename>
point_<DIMS>::point_(point_<3> const& p) : point_{} {
    data_[0] = p.x();
    data_[1] = p.y();
    data_[2] = p.z();
    data_[3] = 1.0_p;
}

/// Indexer
template <size_t DIMS>
precision& point_<DIMS>::operator[](size_t i) {
    return data_[i];
}

template <size_t DIMS>
precision point_<DIMS>::operator[](size_t i) const {
    return data_[i];
}

template <size_t DIMS>
void point_<DIMS>::zero(void) {
    for (size_t r = 0; r < dimensions; r++) {
        data_[r] = 0.0_p;
    }
}

template <size_t DIMS>
void point_<DIMS>::print(std::ostream& os, char const name[]) const {
    os << name << " " << *this << std::endl;
}

template <size_t DIMS>
std::ostream& operator<<(std::ostream& os, point_<DIMS> const& p) {
    os << "point_(";
    for (size_t r = 0; r < p.dimensions; r++) {
        os << p[r] << (r == (p.dimensions - 1) ? "" : ", ");
    }
    os << ")";
    return os;
}

template <size_t DIMS>
point_<DIMS>& point_<DIMS>::operator*=(precision s) {
    for (size_t i = 0; i < dimensions; i++) {
        data_[i] *= s;
    }
    return (*this);
}

template <size_t DIMS>
point_<DIMS>& point_<DIMS>::operator+=(vector_<DIMS> const& a) noexcept(false) {
    for (size_t i = 0; i < DIMS; i++) {
        data_[i] += a[i];
    };
    return (*this);
}

template <size_t DIMS>
point_<DIMS>& point_<DIMS>::operator-=(vector_<DIMS> const& a) noexcept(false) {
    for (size_t i = 0; i < DIMS; i++) {
        data_[i] -= a[i];
    };
    return (*this);
}

// NON-CLASS OPERATIONS
namespace operators {

/// Equality Operator
template <size_t DIMS>
bool operator==(point_<DIMS> const& a, point_<DIMS> const& b) {
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
bool operator!=(point_<DIMS> const& a, point_<DIMS> const& b) {
    return not operator==(a, b);
}

template <size_t DIMS>
point_<DIMS> operator*(linalg::matrix const& a, point_<DIMS> const& b) {
    // basal::exception::throw_unless(a.rows == b.dimensions, __FILE__, __LINE__, "");
    point_<DIMS> c;
    for (size_t y = 0; y < a.rows; y++) {
        for (size_t x = 0; x < a.cols; x++) {
            c[y] += a[y][x] * b[x];
        }
    }
    return c;
}

template <size_t DIMS>
inline bool operator<(point_<DIMS> const& a, point_<DIMS> const& b) noexcept(false) {
    if constexpr (point_<DIMS>::use_distance_sort) {
        vector_<DIMS> a0 = a - point_<DIMS>{};
        vector_<DIMS> b0 = b - point_<DIMS>{};
        return (a0.quadrance() < b0.quadrance());
    }
    if constexpr (point_<DIMS>::use_lexical_sort) {
        for (size_t i = 0; i < DIMS; i++) {
            if (a[i] < b[i]) {
                return true;
            } else if (a[i] > b[i]) {
                return false;
            }
        }
        return false;
    }
    return false;
}

/// Adding points creates a vector
template <size_t DIMS>
vector_<DIMS> operator+(point_<DIMS> const& a, point_<DIMS> const& b) noexcept(false) {
    vector_<DIMS> c;
    for (size_t i = 0; i < c.dimensions; i++) {
        c[i] = a[i] + b[i];
    }
    return c;
}

/// Subtracting points creates a vector
template <size_t DIMS>
vector_<DIMS> operator-(point_<DIMS> const& a, point_<DIMS> const& b) noexcept(false) {
    vector_<DIMS> c;
    for (size_t i = 0; i < c.dimensions; i++) {
        c[i] = a[i] - b[i];
    }
    return c;
}

/// Adding a vector to a point_ creates a new point_
template <size_t DIMS>
point_<DIMS> operator+(point_<DIMS> const& a, const vector_<DIMS>& b) noexcept(false) {
    point_<DIMS> c{a};
    c += b;
    return c;
}

/// Subtracting a vector from a point_ creates a new point_
template <size_t DIMS>
point_<DIMS> operator-(point_<DIMS> const& a, const vector_<DIMS>& b) noexcept(false) {
    point_<DIMS> c{a};
    c -= b;
    return c;
}

}  // namespace operators

template <size_t DIMS>
point_<DIMS> multiply(point_<DIMS> const& a, precision s) noexcept(false) {
    point_<DIMS> c{a};
    c *= s;
    return c;
}

template <size_t DIMS>
point_<DIMS> multiply(precision s, point_<DIMS> const& a) noexcept(false) {
    point_<DIMS> c{a};
    c *= s;
    return c;
}

namespace pairwise {

template <size_t DIMS>
point_<DIMS> multiply(point_<DIMS> const& a, point_<DIMS> const& b) noexcept(false) {
    basal::exception::throw_unless(a.dimensions == b.dimensions, __FILE__, __LINE__, "");
    point_<DIMS> c;
    for (size_t n = 0; n < a.dimensions; n++) {
        c[n] = a[n] * b[n];
    }
    return c;
}

template <size_t DIMS>
point_<DIMS> divide(point_<DIMS> const& a, point_<DIMS> const& b) noexcept(false) {
    basal::exception::throw_unless(a.dimensions == b.dimensions, __FILE__, __LINE__, "");
    point_<DIMS> c;
    for (size_t n = 0; n < a.dimensions; n++) {
        c[n] = a[n] / b[n];
    }
    return c;
}

}  // namespace pairwise

/// Converts a vector to a point_
template <size_t DIM>
point_<DIM> as_point(vector_<DIM> const& v) {
    point_<DIM> P;
    for (size_t i = 0; i < P.dimensions; i++) {
        P[i] = v[i];
    }
    return P;
}

// Explicit Instantiations for 2D
template class point_<2ul>;
template point_<2ul>::point_(precision, precision);
template bool operators::operator== <2ul>(point_<2ul> const&, point_<2ul> const&);
template bool operators::operator!= <2ul>(point_<2ul> const&, point_<2ul> const&);
template bool operators::operator< <2ul>(point_<2ul> const&, point_<2ul> const&) noexcept(false);
template point_<2ul> operators::operator* <2ul>(linalg::matrix const&, point_<2ul> const&);
template vector_<2ul> operators::operator+ <2ul>(point_<2ul> const&, point_<2ul> const&);
template vector_<2ul> operators::operator- <2ul>(point_<2ul> const&, point_<2ul> const&);
template point_<2ul> operators::operator+ <2ul>(point_<2ul> const&, const vector_<2ul>&) noexcept(false);
template point_<2ul> operators::operator- <2ul>(point_<2ul> const&, const vector_<2ul>&) noexcept(false);
template point_<2ul> multiply<2ul>(point_<2ul> const&, double);
template point_<2ul> multiply<2ul>(double, point_<2ul> const&);
template point_<2ul> pairwise::divide<2ul>(point_<2ul> const&, point_<2ul> const&);
template point_<2ul> pairwise::multiply<2ul>(point_<2ul> const&, point_<2ul> const&);
template std::ostream& operator<< <2ul>(std::ostream&, point_<2ul> const&);
template point_<2ul> as_point(vector_<2ul> const&);

// Explicit Instantiations for 3D
template class point_<3ul>;
template point_<3ul>::point_(precision, precision, precision);
template point_<3ul>::point_(point_<2ul> const&);
template bool operators::operator== <3ul>(point_<3ul> const&, point_<3ul> const&);
template bool operators::operator!= <3ul>(point_<3ul> const&, point_<3ul> const&);
template bool operators::operator< <3ul>(point_<3ul> const&, point_<3ul> const&) noexcept(false);
template point_<3ul> operators::operator* <3ul>(linalg::matrix const&, point_<3ul> const&);
template vector_<3ul> operators::operator+ <3ul>(point_<3ul> const&, point_<3ul> const&);
template vector_<3ul> operators::operator- <3ul>(point_<3ul> const&, point_<3ul> const&);
template point_<3ul> operators::operator+ <3ul>(point_<3ul> const&, const vector_<3ul>&) noexcept(false);
template point_<3ul> operators::operator- <3ul>(point_<3ul> const&, const vector_<3ul>&) noexcept(false);
template point_<3ul> multiply<3ul>(point_<3ul> const&, double);
template point_<3ul> multiply<3ul>(double, point_<3ul> const&);
template point_<3ul> pairwise::divide<3ul>(point_<3ul> const&, point_<3ul> const&);
template point_<3ul> pairwise::multiply<3ul>(point_<3ul> const&, point_<3ul> const&);
template std::ostream& operator<< <3ul>(std::ostream&, point_<3ul> const&);
template point_<3ul> as_point(vector_<3ul> const&);

// Explicit Instantiations for 4D
template class point_<4ul>;
template point_<4ul>::point_(precision, precision, precision, precision);
template point_<4ul>::point_(point_<3ul> const&);
template bool operators::operator== <4ul>(point_<4ul> const&, point_<4ul> const&);
template bool operators::operator!= <4ul>(point_<4ul> const&, point_<4ul> const&);
template bool operators::operator< <4ul>(point_<4ul> const&, point_<4ul> const&) noexcept(false);
template point_<4ul> operators::operator* <4ul>(linalg::matrix const&, point_<4ul> const&);
template vector_<4ul> operators::operator+ <4ul>(point_<4ul> const&, point_<4ul> const&);
template vector_<4ul> operators::operator- <4ul>(point_<4ul> const& a, point_<4ul> const& b);
template point_<4ul> operators::operator+ <4ul>(point_<4ul> const& a, const vector_<4ul>& b) noexcept(false);
template point_<4ul> operators::operator- <4ul>(point_<4ul> const& a, const vector_<4ul>& b) noexcept(false);
template point_<4ul> multiply<4ul>(point_<4ul> const&, double);
template point_<4ul> multiply<4ul>(double, point_<4ul> const&);
template point_<4ul> pairwise::divide<4ul>(point_<4ul> const&, point_<4ul> const&);
template point_<4ul> pairwise::multiply<4ul>(point_<4ul> const&, point_<4ul> const&);
template std::ostream& operator<< <4ul>(std::ostream&, point_<4ul> const&);
template point_<4ul> as_point(vector_<4ul> const&);

}  // namespace geometry
