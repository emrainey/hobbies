#pragma once

/// @file
/// The Pixel Class

#include "xmmt/packs.hpp"
#include "xmmt/vector.hpp"

namespace intel {

/// An ordered set of values for a pixel of multiple channels which can be operator-ed on in-bulk.
/// @tparam pack_type The packing structure definition.
template <typename pack_type, size_t _dimensions>
class pixel_ : public pack_type {
protected:
    /// Hold on to the number of elements needed
    constexpr static size_t dimensions = _dimensions;
    /// The data storage of the unit
    static_assert(std::is_floating_point<typename pack_type::element_type>::value,
                  "Must be either float, double or long double");
    static_assert(4 <= pack_type::number_of_elements, "Must be a Intel Parallel/Multiple Data data type");
    static_assert(dimensions <= pack_type::number_of_elements,
                  "The dimension of elements must fit into the number of parallel data slots");
    static_assert(3 == dimensions or 4 == dimensions, "Must have 3 or 4 channels");

public:
    using element_type = typename pack_type::element_type;
    using parallel_type = typename pack_type::parallel_type;

    /// Default Constructor
    constexpr pixel_() : pack_type{} {
    }

    /// Intrinsic Initializer
    constexpr pixel_(parallel_type v) : pack_type{v} {
    }

    /// Forwarding Constructor. This allows the pack_type parameter constructors to be used.
    template <typename... Args>
    pixel_(Args&&... args) : pack_type{std::forward<Args>(args)...} {
    }

    /// Copy Constructor
    pixel_(pixel_ const& other) : pack_type{other.data} {
    }

    /// Move Construction is just a Copy
    pixel_(pixel_&& other) : pack_type{other.data} {
    }

    /// Copy Assignment
    pixel_& operator=(pixel_ const& other) {
        pack_type::operator=(other.data);
        return (*this);
    }

    /// Move Assignment is Copy Assignment
    pixel_& operator=(pixel_&& other) {
        pack_type::operator=(std::move(other.data));
        return (*this);
    }

    /// Destructor
    ~pixel_() = default;

    /// Initializer List Constructor
    constexpr pixel_(std::initializer_list<element_type> list) : pixel_{} {
        size_t n = 0;
        for (auto it = list.begin(); it < list.end() and n < dimensions; it++, n++) {
            pack_type::datum[n] = *it;
        }
    }

    /// The const/volatile indexer operator
    element_type const volatile& operator[](size_t index) const volatile {
        return pack_type::datum[index];
    }

    /// The non-const index operator
    element_type& operator[](size_t index) {
        return pack_type::datum[index];
    }

    /// Scales each value
    inline pixel_& operator*=(element_type a) {
        if constexpr (pack_type::number_of_elements == 2) {
            __m128d tmp = _mm_setr_pd(a, a);
            pack_type::data = _mm_mul_pd(pack_type::data, tmp);
        } else {
            if constexpr (std::is_same_v<element_type, float>) {
                __m128 tmp = _mm_setr_ps(a, a, a, a);
                pack_type::data = _mm_mul_ps(pack_type::data, tmp);
            } else {
                __m256d tmp = _mm256_setr_pd(a, a, a, a);
                pack_type::data = _mm256_mul_pd(pack_type::data, tmp);
            }
        }
        return (*this);
    }

    /// Accumulate value
    inline pixel_& operator+=(pixel_ const& a) {
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

    /// Equality Operator
    bool operator==(pixel_ const& other) {
        bool ret = true;
        for (size_t n = 0; n < dimensions && ret; n++) {
            ret = (pack_type::datum[n] == other[n]);
        }
        return ret;
    }

    /// Inequality Operator
    bool operator!=(pixel_ const& other) {
        return not operator==(other);
    }

    void print(char const name[]) const {
        std::cout << name << " " << (*this) << std::endl;
    }


    inline void clamp(void) {
        if constexpr (pack_type::number_of_elements == 2) {
            parallel_type imm = _mm_set1_pd(1.0_p);
            parallel_type tmp = _mm_min_pd(imm, pack_type::data);
            imm = _mm_setzero_pd();
            pack_type::data = _mm_max_pd(imm, tmp);
        } else {
            if constexpr (std::is_same_v<element_type, float>) {
                parallel_type imm = _mm_set1_ps(1.0_p);
                parallel_type tmp = _mm_min_ps(imm, pack_type::data);
                imm = _mm_setzero_ps();
                pack_type::data = _mm_max_ps(imm, tmp);
            } else {
                parallel_type imm = _mm256_set1_pd(1.0_p);
                parallel_type tmp = _mm256_min_pd(imm, pack_type::data);
                imm = _mm256_setzero_pd();
                pack_type::data = _mm256_max_pd(imm, tmp);
            }
        }
    }

    /****** FRIENDS **************************************************************/


    /// Pixel Scaling
    friend inline pixel_ operator*(pixel_ const& a, element_type const& b) {
        pixel_ c{};
        if constexpr (pack_type::number_of_elements == 2) {
            c.data = _mm_mul_pd(a.data, _mm_set1_pd(b));
        } else {
            if constexpr (std::is_same_v<element_type, float>) {
                c.data = _mm_mul_ps(a.data, _mm_set1_ps(b));
            } else {
                c.data = _mm256_mul_pd(a.data, _mm256_set1_pd(b));
            }
        }
        return c;
    }


    /// Pairwise/Hamard Scaling
    friend inline pixel_ operator*(pixel_ const& a, pixel_ const& b) {
        pixel_ c{};
        if constexpr (pack_type::number_of_elements == 2) {
            c.data = _mm_mul_pd(a.data, b.data);
        } else {
            if constexpr (std::is_same_v<element_type, float>) {
                c.data = _mm_mul_ps(a.data, b.data);
            } else {
                c.data = _mm256_mul_pd(a.data, b.data);
            }
        }
        return c;
    }


    /// @brief Blends two pixels together with linear interpolation
    /// @warning No gamma correction is applied!
    /// @param a
    /// @param b
    /// @param ratio
    /// @return
    friend inline pixel_ interpolate(pixel_ const& a, pixel_ const&b, element_type const ratio) {
        pixel_ c = a * ratio;
        pixel_ d = b * (1.0 - ratio);
        pixel_ e;
        if constexpr (pack_type::number_of_elements == 2) {
            e.data = _mm_add_pd(c.data, d.data);
        } else {
            if constexpr (std::is_same_v<element_type, float>) {
                e.data = _mm_add_ps(c.data, d.data);
            } else {
                e.data = _mm256_add_pd(c.data, d.data);
            }
        }
        return e;
    }

    friend std::ostream& operator<<(std::ostream& os, pixel_ const& a) {
        pack_type const& b = a;
        os << "pixel " << b;
        return os;
    }
};

} // namespace intel
