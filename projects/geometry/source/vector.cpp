
#include "geometry/vector.hpp"

#include "geometry/types.hpp"

namespace geometry {

using namespace operators;

template <size_t DIMS>
vector_<DIMS>::vector_() : data_{0} {
}

template <size_t DIMS>
vector_<DIMS>::vector_(size_t len, precision in[]) : vector_{} {
    basal::exception::throw_if(len == 0, __FILE__, __LINE__);
    basal::exception::throw_if(in == nullptr, __FILE__, __LINE__);
    basal::exception::throw_unless(len == dimensions, __FILE__, __LINE__);
    for (size_t i = 0; i < dimensions and i < len; i++) {
        data_[i] = in[i];
    }
}

template <size_t DIMS>
vector_<DIMS>::vector_(std::initializer_list<precision> a) : vector_{} {
    basal::exception::throw_unless(dimensions == a.size(), __FILE__, __LINE__);
    size_t r = 0;
    for (auto iter = a.begin(); r < a.size() and iter != a.end() and r < dimensions; r++, iter++) {
        data_[r] = *iter;
    }
}

template <size_t DIMS>
vector_<DIMS>::vector_(vector_ const& other) : vector_{} {
    memcpy(data_, other.data_, dimensions * sizeof(precision));
}

template <size_t DIMS>
vector_<DIMS>::vector_(vector_&& other) : vector_{} {
    memcpy(data_, other.data_, dimensions * sizeof(precision));
}

template <size_t DIMS>
vector_<DIMS>& vector_<DIMS>::operator=(vector_ const& other) {
    memcpy(data_, other.data_, dimensions * sizeof(precision));
    return (*this);
}

template <size_t DIMS>
vector_<DIMS>& vector_<DIMS>::operator=(vector_&& other) {
    memcpy(data_, other.data_, dimensions * sizeof(precision));
    return (*this);
}

template <size_t DIMS>
bool vector_<DIMS>::is_zero(void) const {
    for (size_t i = 0; i < dimensions; i++) {
        if (not basal::is_exactly_zero(data_[i])) {
            return false;
        }
    }
    return true;
}

template <size_t DIMS>
precision vector_<DIMS>::magnitude() const {
    statistics::get().magnitudes++;
    if constexpr (DIMS == 2) {
        return std::hypot(data_[0], data_[1]);
    } else if constexpr (DIMS == 3) {
        // this should be supported in C++17 compliant compilers
        return std::hypot(data_[0], data_[1], data_[2]);
    } else {
        return std::sqrt(quadrance());
    }
}

template <size_t DIMS>
precision vector_<DIMS>::norm() const {
    return magnitude();
}

template <size_t DIMS>
precision vector_<DIMS>::quadrance() const {
    precision sum = 0.0_p;
    for (size_t i = 0; i < dimensions; i++) {
        sum += data_[i] * data_[i];
    }
    return sum;
}

template <size_t DIMS>
vector_<DIMS>& vector_<DIMS>::operator*=(precision a) {
    for (size_t i = 0; i < dimensions; i++) {
        data_[i] *= a;
    }
    return (*this);
}

template <size_t DIMS>
vector_<DIMS>& vector_<DIMS>::operator/=(precision a) {
    for (size_t i = 0; i < dimensions; i++) {
        data_[i] /= a;
    }
    return (*this);
}

template <size_t DIMS>
vector_<DIMS>& vector_<DIMS>::operator+=(vector_ const& a) {
    for (size_t i = 0; i < dimensions; i++) {
        data_[i] += a[i];
    }
    return (*this);
}

template <size_t DIMS>
vector_<DIMS>& vector_<DIMS>::operator-=(vector_ const& a) {
    for (size_t i = 0; i < dimensions; i++) {
        data_[i] -= a[i];
    }
    return (*this);
}

template <size_t DIMS>
vector_<DIMS>& vector_<DIMS>::normalize() noexcept(false) {
    precision m = magnitude();
    basal::exception::throw_if(basal::is_exactly_zero(m), __FILE__, __LINE__, "Magnitude is zero, can't normalize");
    return operator/=(m);
}

template <size_t DIMS>
vector_<DIMS> vector_<DIMS>::normalized() const noexcept(false) {
    vector_ v{*this};
    v.normalize();
    return v;
}

template <size_t DIMS>
vector_<DIMS> vector_<DIMS>::operator!() const {
    vector_ v{*this};  // copy
    v *= -1.0_p;
    return v;
}

template <size_t DIMS>
vector_<DIMS> vector_<DIMS>::operator-() const {
    vector_ v{*this};  // copy
    v *= -1.0_p;
    return v;
}

template <size_t DIMS>
vector_<DIMS> vector_<DIMS>::project_along(vector_& a) const {
    precision s = dot((*this), a) / a.quadrance();
    vector_ b{a};
    b *= s;
    return b;
}

template <size_t DIMS>
vector_<DIMS> vector_<DIMS>::project_orthogonal(vector_& a) const {
    vector_ c{*this};
    c -= project_along(a);
    return c;
}

template <size_t DIMS>
void vector_<DIMS>::print(std::ostream& os, char const name[]) const {
    os << name << " vector_{";
    for (size_t i = 0; i < dimensions; i++) {
        os << data_[i] << ((i == dimensions - 1) ? "" : ", ");
    }
    os << "}";
}

// Template functions outside the class
template <size_t DIMS>
iso::radians angle(vector_<DIMS> const& a, vector_<DIMS> const& b) {
    iso::radians r{acos(dot(a, b) / (a.norm() * b.norm()))};
    return r;
}

template <size_t DIMS>
bool orthogonal(vector_<DIMS> const& a, vector_<DIMS> const& b) {
    return basal::nearly_zero(dot(a, b));
}

template <size_t DIMS>
std::ostream& operator<<(std::ostream& os, vector_<DIMS> const& vec) {
    os << "vector_(";
    for (size_t i = 0; i < vec.dimensions; i++) {
        os << vec[i] << (i == (vec.dimensions - 1) ? "" : ",");
    }
    os << ")";
    return os;
}

namespace operators {
template <size_t DIMS>
bool operator|(vector_<DIMS> const& a, vector_<DIMS> const& b) {
    return orthogonal(a, b);
}

template <size_t DIMS>
vector_<DIMS> operator*(matrix const& A, vector_<DIMS> const& b) {
    return multiply(A, b);
}

template <size_t DIMS>
vector_<DIMS> operator+(vector_<DIMS> const& a, vector_<DIMS> const& b) {
    return addition(a, b);
}

template <size_t DIMS>
vector_<DIMS> operator-(vector_<DIMS> const& a, vector_<DIMS> const& b) {
    return subtraction(a, b);
}

template <size_t DIMS>
vector_<DIMS> operator*(vector_<DIMS> const& a, precision b) {
    return multiply(a, b);
}

template <size_t DIMS>
vector_<DIMS> operator*(precision b, vector_<DIMS> const& a) {
    return multiply(a, b);
}

template <size_t DIMS>
vector_<DIMS> operator/(vector_<DIMS> const& a, precision b) {
    return division(a, b);
}

template <size_t DIMS>
vector_<DIMS> operator/(precision b, vector_<DIMS> const& a) {
    return division(a, b);
}

template <size_t DIMS>
vector_<DIMS> operator-(vector_<DIMS> const& a) {
    return negation(a);
}
}  // namespace operators

namespace R2 {
using vector = vector_<2ul>;
}  // namespace R2

namespace R3 {
using vector = vector_<3ul>;

vector cross(vector const& a, vector const& b) noexcept(false) {
    precision x, y, z;
    x = (a[1] * b[2]) - (a[2] * b[1]);
    y = (a[2] * b[0]) - (a[0] * b[2]);
    z = (a[0] * b[1]) - (a[1] * b[0]);
    statistics::get().cross_products++;
    return vector{{x, y, z}};
}

vector nearly_orthogonal(vector const& a) noexcept(false) {
    basal::exception::throw_if(a == R3::null, __FILE__, __LINE__, "Can't find a nearly orthogonal vector to null");
    size_t idx{0U};
    precision value = a[0];
    if (std::abs(a[1]) < value) {
        idx = 1U;
        value = a[1];
    }
    if (std::abs(a[2]) < value) {
        idx = 2U;
        value = a[2];
    }
    vector v{R3::null};
    v[idx] = 1.0_p;  // set the smallest value to 1.0
    return v;
}

bool parallel(vector const& a, vector const& b) {
    return (R3::null == cross(a, b));
}

vector rodrigues(vector const& k, vector const& v, iso::radians theta) {
    vector v_prj_k = dot(v, k) * k;
    vector v_rej_k = v - v_prj_k;
    vector w{cross(k, v)};
    vector v_ort_rot = cos(theta.value) * v_rej_k + sin(theta.value) * cross(k, v_rej_k);
    return v_ort_rot + v_prj_k;
}

precision triple(vector const& u, vector const& v, vector const& w) noexcept(false) {
    vector vw{cross(v, w)};
    return dot(u, vw);
}

}  // namespace R3

namespace R4 {
using vector = vector_<4ul>;
}  // namespace R4

// Explicit Instantiations for 2D

template vector_<2ul>::vector_();
template vector_<2ul>::vector_(size_t, precision[]);
template vector_<2ul>::vector_(std::initializer_list<precision> a);
template vector_<2ul>::vector_(vector_ const&);
template vector_<2ul>::vector_(vector_&&);
template vector_<2ul>& vector_<2ul>::operator=(vector_ const&);
template vector_<2ul>& vector_<2ul>::operator=(vector_&&);
template bool vector_<2ul>::is_zero(void) const;
template precision vector_<2ul>::magnitude() const;
template precision vector_<2ul>::norm() const;
template precision vector_<2ul>::quadrance() const;
template vector_<2ul>& vector_<2ul>::operator*=(precision);
template vector_<2ul>& vector_<2ul>::operator/=(precision);
template vector_<2ul>& vector_<2ul>::operator+=(vector_ const&);
template vector_<2ul>& vector_<2ul>::operator-=(vector_ const&);
template vector_<2ul>& vector_<2ul>::normalize();
template vector_<2ul> vector_<2ul>::normalized() const;
template vector_<2ul> vector_<2ul>::operator!() const;
template vector_<2ul> vector_<2ul>::operator-() const;
template vector_<2ul> vector_<2ul>::project_along(vector_&) const;
template vector_<2ul> vector_<2ul>::project_orthogonal(vector_&) const;
template void vector_<2ul>::print(std::ostream&, char const[]) const;

// Template functions for 2D
template iso::radians angle(vector_<2ul> const&, vector_<2ul> const&);
template bool orthogonal(vector_<2ul> const&, vector_<2ul> const&);
template std::ostream& operator<<(std::ostream&, vector_<2ul> const&);

// Operators for 2D
template bool operators::operator|(vector_<2ul> const&, vector_<2ul> const&);
template vector_<2ul> operators::operator*(matrix const&, vector_<2ul> const&);
template vector_<2ul> operators::operator+(vector_<2ul> const&, vector_<2ul> const&);
template vector_<2ul> operators::operator-(vector_<2ul> const&, vector_<2ul> const&);
template vector_<2ul> operators::operator*(vector_<2ul> const&, precision);
template vector_<2ul> operators::operator*(precision, vector_<2ul> const&);
template vector_<2ul> operators::operator/(vector_<2ul> const&, precision);
template vector_<2ul> operators::operator/(precision, vector_<2ul> const&);
template vector_<2ul> operators::operator-(vector_<2ul> const&);

// Explicit Instantiations for 3D

template vector_<3ul>::vector_();
template vector_<3ul>::vector_(size_t, precision[]);
template vector_<3ul>::vector_(std::initializer_list<precision> a);
template vector_<3ul>::vector_(vector_ const&);
template vector_<3ul>::vector_(vector_&&);
template vector_<3ul>& vector_<3ul>::operator=(vector_ const&);
template vector_<3ul>& vector_<3ul>::operator=(vector_&&);
template bool vector_<3ul>::is_zero(void) const;
template precision vector_<3ul>::magnitude() const;
template precision vector_<3ul>::norm() const;
template precision vector_<3ul>::quadrance() const;
template vector_<3ul>& vector_<3ul>::operator*=(precision);
template vector_<3ul>& vector_<3ul>::operator/=(precision);
template vector_<3ul>& vector_<3ul>::operator+=(vector_ const&);
template vector_<3ul>& vector_<3ul>::operator-=(vector_ const&);
template vector_<3ul>& vector_<3ul>::normalize();
template vector_<3ul> vector_<3ul>::normalized() const;
template vector_<3ul> vector_<3ul>::operator!() const;
template vector_<3ul> vector_<3ul>::operator-() const;
template vector_<3ul> vector_<3ul>::project_along(vector_&) const;
template vector_<3ul> vector_<3ul>::project_orthogonal(vector_&) const;
template void vector_<3ul>::print(std::ostream&, char const[]) const;

// Template functions for 3D
template iso::radians angle(vector_<3ul> const&, vector_<3ul> const&);
template bool orthogonal(vector_<3ul> const&, vector_<3ul> const&);
template std::ostream& operator<<(std::ostream&, vector_<3ul> const&);

// Operators for 3D
template bool operators::operator|(vector_<3ul> const&, vector_<3ul> const&);
template vector_<3ul> operators::operator*(matrix const&, vector_<3ul> const&);
template vector_<3ul> operators::operator+(vector_<3ul> const&, vector_<3ul> const&);
template vector_<3ul> operators::operator-(vector_<3ul> const&, vector_<3ul> const&);
template vector_<3ul> operators::operator*(vector_<3ul> const&, precision);
template vector_<3ul> operators::operator*(precision, vector_<3ul> const&);
template vector_<3ul> operators::operator/(vector_<3ul> const&, precision);
template vector_<3ul> operators::operator/(precision, vector_<3ul> const&);
template vector_<3ul> operators::operator-(vector_<3ul> const&);

// Explicit Instantiations for 4D

template vector_<4ul>::vector_();
template vector_<4ul>::vector_(size_t, precision[]);
template vector_<4ul>::vector_(std::initializer_list<precision> a);
template vector_<4ul>::vector_(vector_ const&);
template vector_<4ul>::vector_(vector_&&);
template vector_<4ul>& vector_<4ul>::operator=(vector_ const&);
template vector_<4ul>& vector_<4ul>::operator=(vector_&&);
template bool vector_<4ul>::is_zero(void) const;
template precision vector_<4ul>::magnitude() const;
template precision vector_<4ul>::norm() const;
template precision vector_<4ul>::quadrance() const;
template vector_<4ul>& vector_<4ul>::operator*=(precision);
template vector_<4ul>& vector_<4ul>::operator/=(precision);
template vector_<4ul>& vector_<4ul>::operator+=(vector_ const&);
template vector_<4ul>& vector_<4ul>::operator-=(vector_ const&);
template vector_<4ul>& vector_<4ul>::normalize();
template vector_<4ul> vector_<4ul>::normalized() const;
template vector_<4ul> vector_<4ul>::operator!() const;
template vector_<4ul> vector_<4ul>::operator-() const;
template vector_<4ul> vector_<4ul>::project_along(vector_&) const;
template vector_<4ul> vector_<4ul>::project_orthogonal(vector_&) const;
template void vector_<4ul>::print(std::ostream&, char const[]) const;

// Template functions for 4D
template iso::radians angle(vector_<4ul> const&, vector_<4ul> const&);
template bool orthogonal(vector_<4ul> const&, vector_<4ul> const&);
template std::ostream& operator<<(std::ostream&, vector_<4ul> const&);

// Operators for 4D
template bool operators::operator|(vector_<4ul> const&, vector_<4ul> const&);
template vector_<4ul> operators::operator*(matrix const&, vector_<4ul> const&);
template vector_<4ul> operators::operator+(vector_<4ul> const&, vector_<4ul> const&);
template vector_<4ul> operators::operator-(vector_<4ul> const&, vector_<4ul> const&);
template vector_<4ul> operators::operator*(vector_<4ul> const&, precision);
template vector_<4ul> operators::operator*(precision, vector_<4ul> const&);
template vector_<4ul> operators::operator/(vector_<4ul> const&, precision);
template vector_<4ul> operators::operator/(precision, vector_<4ul> const&);
template vector_<4ul> operators::operator-(vector_<4ul> const&);

}  // namespace geometry
