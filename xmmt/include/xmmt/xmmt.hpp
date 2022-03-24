
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
    double2() : data(_mm_setzero_pd()) {
    }
    double2(__m128d a) : data(a) {
    }
    constexpr static size_t alignment = 16;
    using parallel_type = __m128d;
    using element_type = double;
    /** define the number of element types within a data unit */
    constexpr static size_t number_of_elements = sizeof(parallel_type) / sizeof(element_type);
    union {
        parallel_type data;
        alignas(alignment) element_type datum[number_of_elements];
        alignas(alignment) struct { double x, y; } c;  // components
    };
};

inline std::ostream& operator<<(std::ostream& os, const double2& a) {
    os << " x=" << a.c.x << ", y=" << a.c.y;
    return os;
}

/** A triplet of doubles */
struct double3 {
public:
    double3() : data(_mm256_setzero_pd()) {
    }
    double3(__m256d a) : data(a) {
    }
    constexpr static size_t alignment = 32;
    using parallel_type = __m256d;
    using element_type = double;
    /** define the number of element types within a data unit */
    constexpr static size_t number_of_elements = sizeof(parallel_type) / sizeof(element_type);
    union {
        parallel_type data;
        alignas(alignment) element_type datum[number_of_elements];
        alignas(alignment) struct { double x, y, z; } c;  // components
    };
};

inline std::ostream& operator<<(std::ostream& os, const double3& a) {
    os << " x=" << a.c.x << ", y=" << a.c.y << ", z=" << a.c.z;
    return os;
}

/** A quad of doubles */
struct double4 {
public:
    double4() : data(_mm256_setzero_pd()) {
    }
    double4(__m256d a) : data(a) {
    }
    constexpr static size_t alignment = 32;
    using parallel_type = __m256d;
    using element_type = double;
    /** define the number of element types within a data unit */
    constexpr static size_t number_of_elements = sizeof(parallel_type) / sizeof(element_type);
    union {
        parallel_type data;
        alignas(alignment) element_type datum[number_of_elements];
        alignas(alignment) struct { double x, y, z, w; } c;  // components
    };
};

inline std::ostream& operator<<(std::ostream& os, const double4& a) {
    os << " x=" << a.c.x << ", y=" << a.c.y << ", z=" << a.c.z << ", w=" << a.c.w;
    return os;
}

/**
 * An ordered set of values which can be operatored on in-bulk.
 * Classically, this is a vector, but it is not used as geometric vector.
 * @tparam pack_type The packing structure definition.
 */
template <typename pack_type, size_t _dimensions>
class xmm_ : public pack_type {
protected:
    /** Hold on to the number of elements needed */
    constexpr static size_t dimensions = _dimensions;
    /** The data storage of the unit */
    static_assert(std::is_floating_point<typename pack_type::element_type>::value,
                  "Must be either float, double or long double");
    static_assert(2 <= pack_type::number_of_elements, "Must be a Intel Parallel/Multiple Data data type");
    static_assert(dimensions <= pack_type::number_of_elements,
                  "The dimension of elements must fit into the number of parallel data slots");
    static_assert(2 <= dimensions and dimensions <= 4, "Must be larger than a single point but smaller than 5");

public:
    xmm_() : pack_type() {
    }

    /** Intrinsic Initializer */
    xmm_(typename pack_type::parallel_type v) : pack_type(v) {
    }

    /** Copy Constructor */
    xmm_(const xmm_& other) : pack_type(other.data) {
    }

    /** Move Construction is just a Copy */
    xmm_(xmm_&& other) : pack_type(other.data) {
    }

    /** Copy Assignment */
    xmm_& operator=(const xmm_& other) {
        pack_type::data = other.data;
    }

    /** Move Assignment is Copy Assignment */
    xmm_& operator=(xmm_&& other) {
        pack_type::data = other.data;
    }

    /** Destructor */
    ~xmm_() = default;

    /** Initializer List Constructor */
    xmm_(std::initializer_list<typename pack_type::element_type> list) : xmm_() {
        size_t n = 0;
        for (auto it = list.begin(); it < list.end() and n < dimensions; it++, n++) {
            pack_type::datum[n] = *it;
        }
    }

    /** The const/volatile indexer operator */
    const volatile typename pack_type::element_type& operator[](size_t index) const volatile {
        return pack_type::datum[index];
    }

    /** The non-const index operator */
    typename pack_type::element_type& operator[](size_t index) {
        return pack_type::datum[index];
    }

    /** Accumulates each value */
    inline xmm_& operator+=(typename pack_type::element_type a) {
        if constexpr (pack_type::number_of_elements == 2) {
            __m128d tmp = _mm_setr_pd(a, a);
            pack_type::data = _mm_add_pd(pack_type::data, tmp);
        } else {
            __m256d tmp = _mm256_setr_pd(a, a, a, a);
            pack_type::data = _mm256_add_pd(pack_type::data, tmp);
        }
        return (*this);
    }

    /** Dissipates each value */
    inline xmm_& operator-=(typename pack_type::element_type a) {
        if constexpr (pack_type::number_of_elements == 2) {
            __m128d tmp = _mm_setr_pd(a, a);
            pack_type::data = _mm_sub_pd(pack_type::data, tmp);
        } else {
            __m256d tmp = _mm256_setr_pd(a, a, a, a);
            pack_type::data = _mm256_sub_pd(pack_type::data, tmp);
        }
        return (*this);
    }

    /** Scales each value */
    inline xmm_& operator*=(typename pack_type::element_type a) {
        if constexpr (pack_type::number_of_elements == 2) {
            __m128d tmp = _mm_setr_pd(a, a);
            pack_type::data = _mm_mul_pd(pack_type::data, tmp);
        } else {
            __m256d tmp = _mm256_setr_pd(a, a, a, a);
            pack_type::data = _mm256_mul_pd(pack_type::data, tmp);
        }
        return (*this);
    }

    /** Exponentiation of each value */
    xmm_& operator^=(typename pack_type::element_type a) {
        for (auto& d : pack_type::datum) {
            d = std::pow(d, a);
        }
        return (*this);
    }

    /** Allows each element to be changed */
    using alteration_iterator =
        std::function<typename pack_type::element_type(size_t, typename pack_type::element_type)>;

    /** Only allows read acces of each element */
    using examination_iterator = std::function<void(size_t, typename pack_type::element_type)>;

    /** Iterator for changing values */
    void for_each(alteration_iterator iter) {
        for (size_t n = 0; n < dimensions; n++) {
            pack_type::datum[n] = iter(n, pack_type::datum[n]);
        }
    }

    /** Iterator for examining values */
    void for_each(examination_iterator iter) const {
        for (size_t n = 0; n < dimensions; n++) {
            iter(n, pack_type::datum[n]);
        }
    }

    /** Scales the vector by the approximate magnitude */
    inline xmm_& normalize() {
        double2 tmp;                          // we'll use this for union access only
        tmp.datum[0] = quadrance();           // put it in the bottom
        tmp.data = _mm_rsqrt14_pd(tmp.data);  // get the inv sqrt approx
        return operator*=(tmp.datum[0]);
    }

    /** Returns the sum of the squares of the elements */
    inline typename pack_type::element_type quadrance() {
        pack_type tmp;
        if constexpr (pack_type::number_of_elements == 2) {
            typename pack_type::parallel_type sqr = _mm_mul_pd(pack_type::data, pack_type::data);
            tmp.data = _mm_hadd_pd(sqr, sqr);
            return tmp.c.x;
        } else {
            typename pack_type::parallel_type sqr = _mm256_mul_pd(pack_type::data, pack_type::data);
            typename pack_type::parallel_type z = _mm256_setzero_pd();
            tmp.data = _mm256_hadd_pd(sqr, z);
            sqr = _mm256_permute4x64_pd(tmp.data, 0xD8);
            tmp.data = _mm256_hadd_pd(sqr, z);
            return tmp.c.x;
        }
    }

    /** Returns the square root of the sum of squares of the elements */
    inline typename pack_type::element_type magnitude() {
        typename pack_type::element_type q = quadrance();
        return sqrt(q);
    }

    inline xmm_& operator+=(const xmm_& a) {
        if constexpr (pack_type::number_of_elements == 2) {
            pack_type::data = _mm_add_pd(pack_type::data, a.data);
        } else {
            pack_type::data = _mm256_add_pd(pack_type::data, a.data);
        }
        return (*this);
    }

    inline xmm_& operator-=(const xmm_& a) {
        if constexpr (pack_type::number_of_elements == 2) {
            pack_type::data = _mm_sub_pd(pack_type::data, a.data);
        } else {
            pack_type::data = _mm256_sub_pd(pack_type::data, a.data);
        }
        return (*this);
    }

    inline xmm_& operator*=(const xmm_& a) {
        if constexpr (pack_type::number_of_elements == 2) {
            pack_type::data = _mm_mul_pd(pack_type::data, a.data);
        } else {
            pack_type::data = _mm256_mul_pd(pack_type::data, a.data);
        }
        return (*this);
    }

    inline xmm_& operator/=(const xmm_& a) {
        if constexpr (pack_type::number_of_elements == 2) {
            pack_type::data = _mm_div_pd(pack_type::data, a.data);
        } else {
            pack_type::data = _mm256_div_pd(pack_type::data, a.data);
        }
        return (*this);
    }

    bool operator==(const xmm_& other) {
        bool ret = true;
        for (size_t n = 0; n < dimensions && ret; n++) {
            ret = (pack_type::datum[n] == other[n]);
        }
        return ret;
    }

    bool operator!=(const xmm_& other) {
        return not operator==(other);
    }

    /****** FRIENDS **************************************************************/

    friend inline xmm_ operator+(const xmm_& a, const xmm_& b) {
        xmm_ c;
        if constexpr (pack_type::number_of_elements == 2) {
            c.data = _mm_add_pd(a.data, b.data);
        } else {
            c.data = _mm256_add_pd(a.data, b.data);
        }
        return c;
    }

    friend inline xmm_ operator*(const xmm_& a, const xmm_& b) {
        xmm_ c;
        if constexpr (pack_type::number_of_elements == 2) {
            c.data = _mm_mul_pd(a.data, b.data);
        } else {
            c.data = _mm256_mul_pd(a.data, b.data);
        }
        return c;
    }

    friend typename pack_type::element_type dot(const xmm_& a, const xmm_& b) {
        typename pack_type::element_type d = 0.0;
        if constexpr (pack_type::number_of_elements == 2) {
            __m128d tmp = _mm_mul_pd(a.data, b.data);
            tmp = _mm_hadd_pd(tmp, tmp);
            xmm_ c(tmp);
            d = c[0];
        } else {
            __m256d t = _mm256_mul_pd(a.data, b.data);
            __m256d z = _mm256_setzero_pd();
            __m256d h = _mm256_hadd_pd(t, z);
            t = _mm256_permute4x64_pd(h, 0xD8);
            h = _mm256_hadd_pd(t, z);
            xmm_ c(h);
            d = c[0];
        }
        return d;
    };

    friend xmm_ cross(const xmm_& a, const xmm_& b) {
        static_assert(dimensions == 3, "Can't preform a cross product on anything but 3d");
        xmm_ c;
        if constexpr (dimensions == 3) {
            __m256d e = _mm256_permute4x64_pd(a.data, 0xC9);
            __m256d f = _mm256_permute4x64_pd(b.data, 0xC9);
            e = _mm256_mul_pd(e, b.data);
            f = _mm256_mul_pd(f, a.data);
            __m256d g = _mm256_sub_pd(f, e);
            c.data = _mm256_permute4x64_pd(g, 0xC9);
            c[3] = 0.0;
        }
        return c;
    }
};

using xmmt2 = xmm_<double2, 2>;
using xmmt3 = xmm_<double3, 3>;
using xmmt4 = xmm_<double4, 4>;

}  // namespace intel
