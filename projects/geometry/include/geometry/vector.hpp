#pragma once
/// @file
/// Definitions for the vector_ object.
/// @copyright Copyright 2020 (C) Erik Rainey.

#include <cstdio>

#include "geometry/types.hpp"

#if defined(__x86_64__)
#include <emmintrin.h>
#include <immintrin.h>
#endif

#include <initializer_list>

using namespace basal::literals;

namespace geometry {

/// A N-dimensional vector_ object
template <typename DATA_TYPE, size_t DIM>
class vector_ {
public:
    static_assert(2 <= DIM and DIM <= 4, "Must have between (inclusive) 2 and 4 dimensions");

    /// The dimensionality of the vector
    constexpr static size_t const dimensions = DIM;

    /// The default constructor
    vector_() : data{} {
    }

    vector_(size_t len, DATA_TYPE in[]) : vector_{} {
        basal::exception::throw_if(len == 0, __FILE__, __LINE__);
        basal::exception::throw_if(in == nullptr, __FILE__, __LINE__);
        basal::exception::throw_unless(len == dimensions, __FILE__, __LINE__);
        for (size_t i = 0; i < dimensions and i < len; i++) {
            data[i] = in[i];
        }
    }

    /// The initializer list constructor, should not be inferred.
    vector_(std::initializer_list<DATA_TYPE> a) : vector_{} {
        basal::exception::throw_unless(dimensions == a.size(), __FILE__, __LINE__);
        size_t r = 0;
        for (auto iter = a.begin(); r < a.size() and iter != a.end(); r++, iter++) {
            data[r] = *iter;
        }
    }

    /// Copy Constructor
    vector_(vector_ const& other) : vector_{} {
        memcpy(data, other.data, dimensions * sizeof(DATA_TYPE));
    }

    /// Move Constructor
    vector_(vector_&& other) : vector_{} {
        memcpy(data, other.data, dimensions * sizeof(DATA_TYPE));
    }

    /// Copy Assignment
    vector_& operator=(vector_ const& other) {
        memcpy(data, other.data, dimensions * sizeof(DATA_TYPE));
        return (*this);
    }

    /// Move Assignment
    vector_& operator=(vector_&& other) {
        memcpy(data, other.data, dimensions * sizeof(DATA_TYPE));
        return (*this);
    }

    /// Deconstructor
    virtual ~vector_() = default;

    /// Indexing operator
    inline DATA_TYPE& operator[](size_t idx) noexcept(false) {
        basal::exception::throw_if(idx >= dimensions, __FILE__, __LINE__);
        return data[idx];
    }

    /// Indexing operator (const)
    inline DATA_TYPE const& operator[](size_t idx) const noexcept(false) {
        basal::exception::throw_if(idx >= dimensions, __FILE__, __LINE__);
        return data[idx];
    }

    /// Indexing operator (volatile)
    inline volatile DATA_TYPE& operator[](size_t idx) volatile noexcept(false) {
        basal::exception::throw_if(idx >= dimensions, __FILE__, __LINE__);
        return data[idx];
    }

    /// Determines if the vector's components are all zero
    inline bool is_zero(void) const {
        for (size_t i = 0; i < dimensions; i++) {
            if (data[i] != 0.0_p or data[i] != -0.0_p) {
                return false;
            }
        }
        return true;
    }

    /// @return The sqrt(quadrance()). Euclidean length of vector.
    virtual DATA_TYPE magnitude() const {
        return sqrt(quadrance());
    }

    /// Another name for the magnitude of the vector
    inline DATA_TYPE norm() const {
        return magnitude();
    }

    /// @return The sum of the squares of the components
    inline DATA_TYPE quadrance() const {
        DATA_TYPE sum = 0.0_p;
        for (size_t i = 0; i < dimensions; i++) {
            sum += data[i] * data[i];
        }
        return sum;
    }

    /// Compute the quadrance of a vector.
    ///\see https://en.wikipedia.org/wiki/Rational_trigonometry#Quadrance
    friend inline DATA_TYPE Q(vector_ const& a) {
        return a.quadrance();
    }

    /// Scales each element
    vector_& operator*=(DATA_TYPE a) {
        for (size_t i = 0; i < dimensions; i++) {
            data[i] *= a;
        }
        return (*this);
    }

    /// Scales each element
    vector_& operator/=(DATA_TYPE a) {
        for (size_t i = 0; i < dimensions; i++) {
            data[i] /= a;
        }
        return (*this);
    }

    /// Adds a scalar to each element
    vector_& operator+=(vector_ const& a) {
        for (size_t i = 0; i < dimensions; i++) {
            data[i] += a[i];
        }
        return (*this);
    }

    /// Subtracts a scalar to each element
    vector_& operator-=(vector_ const& a) {
        for (size_t i = 0; i < dimensions; i++) {
            data[i] -= a[i];
        }
        return (*this);
    }

    /// Normalizes the vector, destructive
    vector_& normalize() noexcept(false) {
        DATA_TYPE m = magnitude();
        basal::exception::throw_if(basal::nearly_zero(m), __FILE__, __LINE__, "Magnitude is zero, can't normalize");
        return operator/=(m);
    }

    /// Normalizes the vector returns a copy
    vector_ normalized() const noexcept(false) {
        vector_ v{*this};
        v.normalize();
        return v;
    }

    /// Returns a negative copy of the vector_
    vector_ operator!() const {
        vector_ v{*this};
        v *= -1.0_p;
        return v;
    }

    /// Creates a scaled vector along the parameter vector
    vector_ project_along(vector_& a) const {
        DATA_TYPE s = dot((*this), a) / a.quadrance();
        vector_ b{a};
        b *= s;
        return b;
    }

    /// Creates a scaled vector orthogonal to the parameter vector
    vector_ project_orthogonal(vector_& a) const {
        vector_ c{*this};
        c -= project_along(a);
        return c;
    }

    inline void print(char const name[]) const {
        printf("%s: vector_(", name);
        for (size_t i = 0; i < dimensions; i++) {
            printf("%lf%s", data[i], (i == dimensions - 1) ? "" : ", ");
        }
        printf(");\r\n");
    }

    /// Returns the dot product of two vectors. Reference parameter form
    friend DATA_TYPE dot(vector_ const& a, vector_ const& b) noexcept(false) {
        DATA_TYPE d = 0.0_p;
        for (size_t i = 0; i < dimensions; i++) {
            d += (a.data[i] * b.data[i]);
        }
        statistics::get().dot_operations++;
        return d;
    }

    /// Computes the spread of two vectors.
    ///\see https://en.wikipedia.org/wiki/Rational_trigonometry#Spread
    friend DATA_TYPE spread(vector_ const& a, vector_ const& b) noexcept(false) {
        basal::exception::throw_unless(a.dimensions >= 2 && a.dimensions < 5, __FILE__, __LINE__, "Must be 2d to 4d");
        // we assume the vectors cross at origin
        // must find the perpendicular line from a to b.
        if (a.dimensions == 2) {
            return (a[0] * b[1] - b[0] * a[1]) / (Q(a) * Q(b));
        } else {
            precision d = dot(a, b);
            return 1.0_p - ((d * d) / (Q(a) * Q(b)));
        }
    }

    /// Equality operator of two vector
    friend bool operator==(vector_ const& a, vector_ const& b) {
        for (size_t i = 0; i < dimensions; i++) {
            if (not basal::nearly_equals(a.data[i], b.data[i])) {
                return false;
            }
        }
        return true;
    }

    friend bool operator!=(vector_ const& a, vector_ const& b) {
        return not operator==(a, b);
    }

    friend inline vector_ multiply(vector_ const& a, DATA_TYPE b) {
        vector_ c{a};
        c *= b;
        return c;
    }

    friend inline vector_ division(vector_ const& a, DATA_TYPE b) {
        return multiply(a, 1.0_p / b);
    }

    friend inline vector_ multiply(DATA_TYPE b, vector_ const& a) {
        return multiply(a, b);
    }

    friend inline vector_ division(DATA_TYPE b, vector_ const& a) {
        return division(a, b);
    }

    friend inline vector_ addition(vector_ const& a, vector_ const& b) {
        vector_ c;
        for (size_t i = 0; i < dimensions; i++) {
            c.data[i] = a.data[i] + b.data[i];
        }
        return c;
    }

    friend inline vector_ subtraction(vector_ const& a, vector_ const& b) {
        vector_ c;
        for (size_t i = 0; i < dimensions; i++) {
            c.data[i] = a.data[i] - b.data[i];
        }
        return c;
    }

    friend vector_ multiply(matrix const& A, vector_ const& b) {
        basal::exception::throw_unless(A.cols == b.dimensions, __FILE__, __LINE__);
        vector_ c;
        for (size_t j = 0; j < A.rows; j++) {
            precision d = 0.0_p;
            for (size_t i = 0; i < A.cols; i++) {
                d += A[j][i] * b[i];
            }
            c[j] = d;
        }
        return c;
    }

    friend vector_ negation(vector_ const& a) {
        vector_ b{a};
        b *= -1.0_p;
        return b;
    }

protected:
    // the data type array (prealigned for optimizations)
    alignas(16) DATA_TYPE data[dimensions];
};

/// Computes the angle between two vectors in radians
template <typename DATA_TYPE, size_t DIMS>
iso::radians angle(vector_<DATA_TYPE, DIMS> const& a, vector_<DATA_TYPE, DIMS> const& b) {
    iso::radians r{acos(dot(a, b) / (a.norm() * b.norm()))};
    return r;
}

/// Tests if two vectors are orthagonal
template <typename DATA_TYPE, size_t DIMS>
inline bool orthogonal(vector_<DATA_TYPE, DIMS> const& a, vector_<DATA_TYPE, DIMS> const& b) {
    return basal::nearly_zero(dot(a, b));
}

namespace operators {
/// Tests if two vectors are orthagonal
template <typename DATA_TYPE, size_t DIMS>
inline bool operator|(vector_<DATA_TYPE, DIMS> const& a, vector_<DATA_TYPE, DIMS> const& b) {
    return orthogonal(a, b);
}

template <typename DATA_TYPE, size_t DIMS>
inline vector_<DATA_TYPE, DIMS> operator*(matrix const& A, vector_<DATA_TYPE, DIMS> const& b) {
    return multiply(A, b);
}

template <typename DATA_TYPE, size_t DIMS>
inline vector_<DATA_TYPE, DIMS> operator+(vector_<DATA_TYPE, DIMS> const& a, vector_<DATA_TYPE, DIMS> const& b) {
    return addition(a, b);
}

template <typename DATA_TYPE, size_t DIMS>
inline vector_<DATA_TYPE, DIMS> operator-(vector_<DATA_TYPE, DIMS> const& a, vector_<DATA_TYPE, DIMS> const& b) {
    return subtraction(a, b);
}

template <typename DATA_TYPE, size_t DIMS>
inline vector_<DATA_TYPE, DIMS> operator*(vector_<DATA_TYPE, DIMS> const& a, precision b) {
    return multiply(a, b);
}

template <typename DATA_TYPE, size_t DIMS>
inline vector_<DATA_TYPE, DIMS> operator*(precision b, vector_<DATA_TYPE, DIMS> const& a) {
    return multiply(a, b);
}

template <typename DATA_TYPE, size_t DIMS>
inline vector_<DATA_TYPE, DIMS> operator/(vector_<DATA_TYPE, DIMS> const& a, precision b) {
    return division(a, b);
}

template <typename DATA_TYPE, size_t DIMS>
inline vector_<DATA_TYPE, DIMS> operator/(precision b, vector_<DATA_TYPE, DIMS> const& a) {
    return division(a, b);
}

template <typename DATA_TYPE, size_t DIMS>
inline vector_<DATA_TYPE, DIMS> operator-(vector_<DATA_TYPE, DIMS> const& a) {
    return negation(a);
}
}  // namespace operators

/// Printing Operator
template <typename DATA_TYPE, size_t DIMS>
std::ostream& operator<<(std::ostream& os, vector_<DATA_TYPE, DIMS> const& vec) {
    os << "vector_(";
    for (size_t i = 0; i < vec.dimensions; i++) {
        os << vec[i] << (i == (vec.dimensions - 1) ? "" : ",");
    }
    os << ")";
    return os;
}

/// 2D Space
namespace R2 {
using vector = vector_<precision, 2>;

static vector const null{{0.0_p, 0.0_p}};
static vector const nan{{static_cast<precision>(basal::nan), static_cast<precision>(basal::nan)}};
/// Predefine the axis vectors to prevent constant redefinitions in tests and other code
namespace basis {
static vector const X{{1.0_p, 0.0_p}};
static vector const Y{{0.0_p, 1.0_p}};
}  // namespace basis
}  // namespace R2

/// 3D Space
namespace R3 {
using vector = vector_<precision, 3>;

static vector const null{{0.0_p, 0.0_p, 0.0_p}};
static vector const nan{{static_cast<precision>(basal::nan), static_cast<precision>(basal::nan), static_cast<precision>(basal::nan)}};
/// Predefine the axis vectors to prevent constant redefinitions in tests and other code
namespace basis {
static vector const X{{1.0_p, 0.0_p, 0.0_p}};
static vector const Y{{0.0_p, 1.0_p, 0.0_p}};
static vector const Z{{0.0_p, 0.0_p, 1.0_p}};
}  // namespace basis

/// Returns a new vector which is the cross Product of A and B vectors (i.e. A cross B). Copy parameter form.
vector cross(vector const& a, vector const& b) noexcept(false);

/// Tests if two vectors are parallel
bool parallel(vector const& a, vector const& b);

/// Rotates a vector v around the axis vector k by theta radians.
vector rodrigues(vector const& k, vector const& v, iso::radians theta);

/// Computes the triple product of three vectors
precision triple(vector const& u, vector const& v, vector const& w) noexcept(false);
}  // namespace R3

/// 4D Space
namespace R4 {
using vector = vector_<precision, 4>;

static vector const null{{0.0_p, 0.0_p, 0.0_p, 0.0_p}};
static vector const nan{{static_cast<precision>(basal::nan), static_cast<precision>(basal::nan), static_cast<precision>(basal::nan), static_cast<precision>(basal::nan)}};
/// Predefine the axis vectors to prevent constant redefinitions in tests and other code
namespace basis {
static vector const X{{1.0_p, 0.0_p, 0.0_p, 0.0_p}};
static vector const Y{{0.0_p, 1.0_p, 0.0_p, 0.0_p}};
static vector const Z{{0.0_p, 0.0_p, 1.0_p, 0.0_p}};
static vector const W{{0.0_p, 0.0_p, 0.0_p, 1.0_p}};
}  // namespace basis
}  // namespace R4

namespace operators {
/// Tests if two vectors are parallel
template <typename DATA_TYPE, size_t DIMS>
inline bool operator||(vector_<DATA_TYPE, DIMS> const& a, vector_<DATA_TYPE, DIMS> const& b) {
    static_assert(DIMS == 3, "Can only compute parallelism in 3D");
    return R3::parallel(a, b);
}
}  // namespace operators

}  // namespace geometry
