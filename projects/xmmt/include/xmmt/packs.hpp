#pragma once

/**
 * @file
 * The AVX2 Packed Structures which are in the compact representation we need.
 */

#include <immintrin.h>

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <iostream>
#include <typeinfo>

namespace intel {

/** A pair of doubles */
struct double2 {
public:
    /** The parallel data type used */
    using parallel_type = __m128d;
    /** The element data type used */
    using element_type = double;
    /** The required alignment for this type */
    constexpr static size_t alignment = 16;
    /** define the number of element types within a data unit */
    constexpr static size_t number_of_elements = sizeof(parallel_type) / sizeof(element_type);
    /** Default Constructor */
    double2() : data{_mm_setzero_pd()} {
    }
    /** Forwarding Element Parameter Constructor */
    double2(element_type a, element_type b) {
        c.x = a;
        c.y = b;
    }
    /** Parallel Parameter Constructor */
    double2(parallel_type a) : data{a} {
    }

    /** Sets data to zero */
    inline void zero(void) {
        data = _mm_setzero_pd();
    }

    /** Returns the floored value */
    inline parallel_type floor() const {
        return _mm_floor_pd(data);
    }

    /** Returns the fractional value in the parallel type */
    inline parallel_type fract() const {
        parallel_type tmp = _mm_floor_pd(data);
        return _mm_sub_pd(data, tmp);
    }

    union {
        /** The parallel data type */
        parallel_type data;
        /** The array data type */
        alignas(alignment) element_type datum[number_of_elements];
        /** The named data type */
        alignas(alignment) struct {
            double x, y;
        } c;  // components
    };
};

inline std::ostream& operator<<(std::ostream& os, const double2& a) {
    os << " x=" << a.c.x << ", y=" << a.c.y;
    return os;
}

/** A triplet of doubles */
struct double3 {
public:
    /** The parallel data type used */
    using parallel_type = __m256d;
    /** The element data type used */
    using element_type = double;
    /** The required alignment for this type */
    constexpr static size_t alignment = 32;
    /** define the number of element types within a data unit */
    constexpr static size_t number_of_elements = sizeof(parallel_type) / sizeof(element_type);
    /** Default Constructor */
    double3() : data{_mm256_setzero_pd()} {
    }
    /** Forwarding Element Parameter Constructor */
    double3(element_type a, element_type b, element_type d) {
        c.x = a;
        c.y = b;
        c.z = d;
    }
    /** Parallel Parameter Constructor */
    double3(parallel_type a) : data{a} {
    }

    /** Sets data to zero */
    inline void zero(void) {
        data = _mm256_setzero_pd();
    }

    /** Returns the floored value */
    inline parallel_type floor() const {
        return _mm256_floor_pd(data);
    }

    /** Returns the fractional value in the parallel type */
    inline parallel_type fract() const {
        parallel_type tmp = _mm256_floor_pd(data);
        return _mm256_sub_pd(data, tmp);
    }

    union {
        /** The parallel data type */
        parallel_type data;
        /** The array data type */
        alignas(alignment) element_type datum[number_of_elements];
        /** The named data type */
        alignas(alignment) struct {
            double x, y, z;
        } c;  // components
    };
};

inline std::ostream& operator<<(std::ostream& os, const double3& a) {
    os << " x=" << a.c.x << ", y=" << a.c.y << ", z=" << a.c.z;
    return os;
}

/** A quad of doubles */
struct double4 {
public:
    /** The parallel data type used */
    using parallel_type = __m256d;
    /** The element data type used */
    using element_type = double;
    /** The required alignment for this type */
    constexpr static size_t alignment = 32;
    /** define the number of element types within a data unit */
    constexpr static size_t number_of_elements = sizeof(parallel_type) / sizeof(element_type);
    /** Default Constructor */
    double4() : data{_mm256_setzero_pd()} {
    }
    /** Forwarding Element Parameter Constructor */
    double4(element_type a, element_type b, element_type d, element_type e) {
        c.x = a;
        c.y = b;
        c.z = d;
        c.w = e;
    }
    /** Parallel Parameter Constructor */
    double4(parallel_type a) : data{a} {
    }

    /** Sets data to zero */
    inline void zero(void) {
        data = _mm256_setzero_pd();
    }

    /** Returns a new point which is the floored of this point */
    inline parallel_type floor() const {
        return _mm256_floor_pd(data);
    }

    /** Returns the fractional value in the parallel type */
    inline parallel_type fract() const {
        parallel_type tmp = _mm256_floor_pd(data);
        return _mm256_sub_pd(data, tmp);
    }

    union {
        /** The parallel data type */
        parallel_type data;
        /** The array data type */
        alignas(alignment) element_type datum[number_of_elements];
        /** The named data type */
        alignas(alignment) struct {
            double x, y, z, w;
        } c;  // components
    };
};

inline std::ostream& operator<<(std::ostream& os, const double4& a) {
    os << " x=" << a.c.x << ", y=" << a.c.y << ", z=" << a.c.z << ", w=" << a.c.w;
    return os;
}

}  // namespace intel
