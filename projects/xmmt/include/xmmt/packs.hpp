#pragma once
/// @file
/// The SSE2/AVX2 Packed Structures which are in the compact representation we need.

#include "xmmt/pragmas.hpp"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <iostream>
#include <typeinfo>

#include <basal/ieee754.hpp>
using namespace basal::literals;

namespace intel {

#if defined(__SSE__)
/// A triplet of floats
struct float3 {
public:
    /// The parallel data type used
    using parallel_type = __m128;
    /// The element data type used
    using element_type = float;
    /// The other element type not used
    using other_type = double;
    /// The desired number of units (which may be smaller than number of elements)
    constexpr static size_t desired_elements = 3;
    /// The required alignment for this type
    constexpr static size_t alignment = 16;
    /// define the number of element types within a data unit
    constexpr static size_t number_of_elements = sizeof(parallel_type) / sizeof(element_type);
    /// Default Constructor
    constexpr float3() : float3{0.0f, 0.0f, 0.0f} {
    }
    /// Forwarding Element Parameter Constructor
    constexpr explicit float3(element_type a, element_type b, element_type d) : datum{a, b, d, 1.0_p} {
    }
    /// Forwarding Element Parameter Constructor
    constexpr explicit float3(other_type a, other_type b, other_type d)
        : float3{static_cast<element_type>(a), static_cast<element_type>(b), static_cast<element_type>(d)} {
    }
    /// Parallel Parameter Constructor
    float3(parallel_type a) : data{a} {
    }
    /// Copy Constructor
    float3(float3 const& other) : data{other.data} {
    }
    /// Move Constructor
    float3(float3&& other) : data{other.data} {
        other.zero();
    }
    /// Copy Assign
    float3& operator=(float3 const& other) {
        data = other.data;
        return (*this);
    }
    /// Move Assign
    float3& operator=(float3&& other) {
        data = other.data;
        other.zero();
        return (*this);
    }
    /// Sets data to zero
    inline void zero(void) {
        data = _mm_setzero_ps();
    }

    /// Indexing into the datums
    inline element_type operator[](size_t const& index) const {
        if (index < number_of_elements) {
            return datum[index];
        } else {
            return datum[index % number_of_elements];
        }
    }

    /// Indexing into the datums
    inline element_type& operator[](size_t const& index) {
        if (index < number_of_elements) {
            return datum[index];
        } else {
            return datum[index % number_of_elements];
        }
    }

    union {
        /// The parallel data type
        alignas(alignment) parallel_type data;
        /// The array data type
        alignas(alignment) element_type datum[number_of_elements];
        /// The named data type
        alignas(alignment) struct {
            element_type x, y, z;
        } c;  // components
    };
};

inline std::ostream& operator<<(std::ostream& os, float3 const& a) {
    os << " x=" << a.c.x << ", y=" << a.c.y << ", z=" << a.c.z;
    return os;
}
#endif

#if defined(__SSE__)
/// A quad of floats
struct float4 {
public:
    /// The parallel data type used
    using parallel_type = __m128;
    /// The element data type used
    using element_type = float;
    /// The other element type not used
    using other_type = double;
    /// The desired number of units (which may be smaller than number of elements)
    constexpr static size_t desired_elements = 4;
    /// The required alignment for this type
    constexpr static size_t alignment = 32;
    /// define the number of element types within a data unit
    constexpr static size_t number_of_elements = sizeof(parallel_type) / sizeof(element_type);
    /// Default Constructor
    constexpr float4() : float4{0.0f, 0.0f, 0.0f, 0.0f} {
    }
    /// Forwarding Element Parameter Constructor
    constexpr explicit float4(element_type a, element_type b, element_type d, element_type e) : datum{a, b, d, e} {
        // _mm_set_ps(a, b, d, e);
    }
    /// Forwarding Element Parameter Constructor
    constexpr explicit float4(other_type a, other_type b, other_type d, other_type e)
        : float4{static_cast<element_type>(a), static_cast<element_type>(b), static_cast<element_type>(d),
                 static_cast<element_type>(e)} {
    }
    /// Parallel Parameter Constructor
    float4(parallel_type a) : data{a} {
    }
    /// Copy Constructor
    float4(float4 const& other) : data{other.data} {
    }
    /// Move Constructor
    float4(float4&& other) : data{other.data} {
        other.zero();
    }
    /// Copy Assign
    float4& operator=(float4 const& other) {
        data = other.data;
        return (*this);
    }
    /// Move Assign
    float4& operator=(float4&& other) {
        data = other.data;
        other.zero();
        return (*this);
    }
    /// Sets data to zero
    inline void zero(void) {
        data = _mm_setzero_ps();
    }

    union {
        /// The parallel data type
        alignas(alignment) parallel_type data;
        /// The array data type
        alignas(alignment) element_type datum[number_of_elements];
        /// The named data type
        alignas(alignment) struct {
            element_type x, y, z, w;
        } c;  // components
    };
};

inline std::ostream& operator<<(std::ostream& os, float4 const& a) {
    os << " x=" << a.c.x << ", y=" << a.c.y << ", z=" << a.c.z << ", w=" << a.c.w;
    return os;
}
#endif

#if defined(__SSE__)
/// A pair of doubles
struct double2 {
public:
    /// The parallel data type used
    using parallel_type = __m128d;
    /// The element data type used
    using element_type = double;
    /// The other element type not used
    using other_type = float;
    /// The desired number of units (which may be smaller than number of elements)
    constexpr static size_t desired_elements = 2;
    /// The required alignment for this type
    constexpr static size_t alignment = 16;
    /// define the number of element types within a data unit
    constexpr static size_t number_of_elements = sizeof(parallel_type) / sizeof(element_type);
    /// Default Constructor
    constexpr double2() : double2{0.0, 0.0} {
    }
    /// Forwarding Element Parameter Constructor
    constexpr double2(element_type a, element_type b) : datum{a, b} {
    }
    /// Forwarding Element Parameter Constructor
    constexpr double2(other_type a, other_type b)
        : double2{static_cast<element_type>(a), static_cast<element_type>(b)} {
    }
    /// Parallel Parameter Constructor
    double2(parallel_type a) : data{a} {
    }
    /// Copy Constructor
    double2(double2 const& other) : data{other.data} {
    }
    /// Move Constructor
    double2(double2&& other) : data{other.data} {
        other.zero();
    }
    /// Copy Assign
    double2& operator=(double2 const& other) {
        data = other.data;
        return (*this);
    }
    /// Move Assign
    double2& operator=(double2&& other) {
        data = other.data;
        other.zero();
        return (*this);
    }
    /// Sets data to zero
    inline void zero(void) {
        data = _mm_setzero_pd();
    }
    /// Indexing into the datums
    inline element_type& operator[](size_t const& index) {
        if (index < number_of_elements) {
            return datum[index];
        } else {
            return datum[index % number_of_elements];
        }
    }
    union {
        /// The parallel data type
        alignas(alignment) parallel_type data;
        /// The array data type
        alignas(alignment) element_type datum[number_of_elements];
        /// The named data type
        alignas(alignment) struct {
            element_type x, y;
        } c;  // components
    };
};

inline std::ostream& operator<<(std::ostream& os, double2 const& a) {
    os << " x=" << a.c.x << ", y=" << a.c.y;
    return os;
}
#endif

#if defined(__AVX2__)
/// A triplet of doubles
struct double3 {
public:
    /// The parallel data type used
    using parallel_type = __m256d;
    /// The element data type used
    using element_type = double;
    /// The other element type not used
    using other_type = float;
    /// The desired number of units (which may be smaller than number of elements)
    constexpr static size_t desired_elements = 3;
    /// The required alignment for this type
    constexpr static size_t alignment = 32;
    /// define the number of element types within a data unit
    constexpr static size_t number_of_elements = sizeof(parallel_type) / sizeof(element_type);
    /// Default Constructor
    constexpr double3() : double3{0.0, 0.0, 0.0} {
    }
    /// Forwarding Element Parameter Constructor
    constexpr explicit double3(element_type a, element_type b, element_type d) : datum{a, b, d, 1.0_p} {
        // _mm256_set_pd(a, b, d, 1.0_p);
    }
    /// Forwarding Element Parameter Constructor
    constexpr explicit double3(other_type a, other_type b, other_type d)
        : double3{static_cast<element_type>(a), static_cast<element_type>(b), static_cast<element_type>(d)} {
    }

    /// Parallel Parameter Constructor
    double3(parallel_type a) : data{a} {
    }
    /// Copy Constructor
    double3(double3 const& other) : data{other.data} {
    }
    /// Move Constructor
    double3(double3&& other) : data{other.data} {
        other.zero();
    }
    /// Copy Assign
    double3& operator=(double3 const& other) {
        data = other.data;
        return (*this);
    }
    /// Move Assign
    double3& operator=(double3&& other) {
        data = other.data;
        other.zero();
        return (*this);
    }
    /// Sets data to zero
    inline void zero(void) {
        data = _mm256_setzero_pd();
    }

    union {
        /// The parallel data type
        alignas(alignment) parallel_type data;
        /// The array data type
        alignas(alignment) element_type datum[number_of_elements];
        /// The named data type
        alignas(alignment) struct {
            element_type x, y, z;
        } c;  // components
    };
};

inline std::ostream& operator<<(std::ostream& os, double3 const& a) {
    os << " x=" << a.c.x << ", y=" << a.c.y << ", z=" << a.c.z;
    return os;
}
#endif

#if defined(__AVX2__)
/// A quad of doubles
struct double4 {
public:
    /// The parallel data type used
    using parallel_type = __m256d;
    /// The element data type used
    using element_type = double;
    /// The other element type not used
    using other_type = float;
    /// The desired number of units (which may be smaller than number of elements)
    constexpr static size_t desired_elements = 4;
    /// The required alignment for this type
    constexpr static size_t alignment = 32;
    /// define the number of element types within a data unit
    constexpr static size_t number_of_elements = sizeof(parallel_type) / sizeof(element_type);
    /// Default Constructor
    constexpr double4() : double4{0.0, 0.0, 0.0, 0.0} {
    }
    /// Forwarding Element Parameter Constructor
    constexpr explicit double4(element_type a, element_type b, element_type d, element_type e) : datum{a, b, d, e} {
    }
    /// Forwarding Element Parameter Constructor
    constexpr explicit double4(other_type a, other_type b, other_type d, other_type e)
        : double4{static_cast<element_type>(a), static_cast<element_type>(b), static_cast<element_type>(d),
                  static_cast<element_type>(e)} {
    }
    /// Parallel Parameter Constructor
    double4(parallel_type a) : data{a} {
    }
    /// Copy Constructor
    double4(double4 const& other) : data{other.data} {
    }
    /// Move Constructor
    double4(double4&& other) : data{other.data} {
        other.zero();
    }
    /// Copy Assign
    double4& operator=(double4 const& other) {
        data = other.data;
        return (*this);
    }
    /// Move Assign
    double4& operator=(double4&& other) {
        data = other.data;
        other.zero();
        return (*this);
    }

    /// Sets data to zero
    inline void zero(void) {
        data = _mm256_setzero_pd();
    }

    union {
        /// The parallel data type
        alignas(alignment) parallel_type data;
        /// The array data type
        alignas(alignment) element_type datum[number_of_elements];
        /// The named data type
        alignas(alignment) struct {
            element_type x, y, z, w;
        } c;  // components
    };
};

inline std::ostream& operator<<(std::ostream& os, double4 const& a) {
    os << " x=" << a.c.x << ", y=" << a.c.y << ", z=" << a.c.z << ", w=" << a.c.w;
    return os;
}
#endif
}  // namespace intel
