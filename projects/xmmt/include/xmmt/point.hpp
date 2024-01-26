#pragma once

/**
 * @file
 * The Point Class
 */

#include "xmmt/packs.hpp"
#include "xmmt/vector.hpp"

namespace intel {

/**
 * An ordered set of values for a geometric point which can be operator-ed on in-bulk.
 * @tparam pack_type The packing structure definition.
 */
template <typename pack_type, size_t _dimensions>
class point_ : public pack_type {
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
    using element_type = typename pack_type::element_type;
    using parallel_type = typename pack_type::parallel_type;

    /** Default Constructor */
    constexpr point_() : pack_type{} {
    }

    /** Intrinsic Initializer */
    point_(parallel_type v) : pack_type{v} {
    }

    /** Forwarding Constructor */
    template <typename... Args>
    point_(Args&&... args) : pack_type{std::forward<Args>(args)...} {
    }

    /** Copy Constructor */
    point_(const point_& other) : pack_type{other.data} {
    }

    /** Move Construction is just a Copy */
    point_(point_&& other) : pack_type{other.data} {
    }

    /** Copy Assignment */
    point_& operator=(const point_& other) {
        pack_type::data = other.data;
    }

    /** Move Assignment is Copy Assignment */
    point_& operator=(point_&& other) {
        pack_type::data = other.data;
    }

    /** Destructor */
    ~point_() = default;

    /** Initializer List Constructor */
    point_(std::initializer_list<element_type> list) : point_{} {
        size_t n = 0;
        for (auto it = list.begin(); it < list.end() and n < dimensions; it++, n++) {
            pack_type::datum[n] = *it;
        }
    }

    /** The const/volatile indexer operator */
    const volatile element_type& operator[](size_t index) const volatile {
        return pack_type::datum[index];
    }

    /** The non-const index operator */
    element_type& operator[](size_t index) {
        return pack_type::datum[index];
    }

    // /** Allows each element to be changed */
    // using alteration_iterator
    //     = std::function<element_type(size_t, element_type)>;

    // /** Only allows read access of each element */
    // using examination_iterator = std::function<void(size_t, element_type)>;

    // /** Iterator for changing values */
    // void for_each (alteration_iterator iter) {
    //     for (size_t n = 0; n < dimensions; n++) {
    //         pack_type::datum[n] = iter(n, pack_type::datum[n]);
    //     }
    // }

    // /** Iterator for examining values */
    // void for_each (examination_iterator iter) const {
    //     for (size_t n = 0; n < dimensions; n++) {
    //         iter(n, pack_type::datum[n]);
    //     }
    // }

    /** Equality Operator */
    bool operator==(const point_& other) {
        bool ret = true;
        for (size_t n = 0; n < dimensions && ret; n++) {
            ret = (pack_type::datum[n] == other[n]);
        }
        return ret;
    }

    /** Inequality Operator */
    bool operator!=(const point_& other) {
        return not operator==(other);
    }

    void print(const char name[]) const {
        std::cout << name << " " << (*this) << std::endl;
    }

    /** A point plus a vector is a moved point */
    inline point_& operator+=(const vector_<pack_type, dimensions>& a) {
        if constexpr (pack_type::number_of_elements == 2) {
            pack_type::data = _mm_add_pd(pack_type::data, a.data);
        } else {
            if constexpr (std::is_same_v<element_type, float>) {
                pack_type::data = _mm_add_ps(pack_type::data, a.data);
            } else {
                pack_type::data = _mm256_add_pd(pack_type::data, a.data);
            }
        }
        return (*this);
    }

    /** A point minus a vector is a moved point */
    inline point_& operator-=(const vector_<pack_type, dimensions>& a) {
        if constexpr (pack_type::number_of_elements == 2) {
            pack_type::data = _mm_sub_pd(pack_type::data, a.data);
        } else {
            if constexpr (std::is_same_v<element_type, float>) {
                pack_type::data = _mm_sub_ps(pack_type::data, a.data);
            } else {
                pack_type::data = _mm256_sub_pd(pack_type::data, a.data);
            }
        }
        return (*this);
    }

    /****** FRIENDS **************************************************************/

    friend inline point_ floor(point_ const& a) {
        point_ c{};
        if constexpr (pack_type::number_of_elements == 2) {
            c.data = _mm_floor_pd(a.data);
        } else {
            if constexpr (std::is_same_v<element_type, float>) {
                c.data = _mm_floor_ps(a.data);
            } else {
                c.data = _mm256_floor_pd(a.data);
            }
        }
        return c;
    }

    friend inline point_ fract(point_ const& a) {
        point_ c{};
        if constexpr (pack_type::number_of_elements == 2) {
            parallel_type tmp = _mm_floor_pd(a.data);
            c.data = _mm_sub_pd(a.data, tmp);
        } else {
            if constexpr (std::is_same_v<element_type, float>) {
                parallel_type tmp = _mm_floor_ps(a.data);
                c.data = _mm_sub_ps(a.data, tmp);
            } else {
                parallel_type tmp = _mm256_floor_pd(a.data);
                c.data = _mm256_sub_pd(a.data, tmp);
            }
        }
        return c;
    }

    /** A Point minus a point is a vector from the last to the first. */
    // friend vector_ operator-(const point_& a, const point_& b) {
    //     vector_ c{};
    //     if constexpr (point_::number_of_elements == 2) {
    //         c.data = _mm_sub_pd(a.data, b.data);
    //     } else {
    //         if constexpr (std::is_same_v<typename pack_type::element_type, float>) {
    //             c.data = _mm_sub_ps(a.data, b.data);
    //         } else {
    //             c.data = _mm256_sub_pd(a.data, b.data);
    //         }
    //     }
    //     return c;
    // }

    /** A Point plus a vector is a point. */
    friend point_ operator+(const point_& a, const vector_<pack_type, dimensions>& b) {
        point_ c{};
        if constexpr (point_::number_of_elements == 2) {
            c.data = _mm_add_pd(a.data, b.data);
        } else {
            if constexpr (std::is_same_v<typename pack_type::element_type, float>) {
                c.data = _mm_add_ps(a.data, b.data);
            } else {
                c.data = _mm256_add_pd(a.data, b.data);
            }
        }
        return c;
    }

    friend std::ostream& operator<<(std::ostream& os, const point_& a) {
        pack_type const& b = a;
        os << "point " << b;
        return os;
    }
};

// namespace constants {
// namespace R2 {
// constexpr static point_<float2, 2ul> zero;
// constexpr static point_<double2, 2ul> zero;
// }  // namespace R2
// namespace R3 {
// constexpr static point_<float3, 3ul> zero;
// constexpr static point_<double3, 3ul> zero;
// }  // namespace R3
// namespace R4 {
// constexpr static point_<float4, 4ul> zero;
// constexpr static point_<double4, 4ul> zero;
// }  // namespace R4
// }  // namespace constants

}  // namespace intel