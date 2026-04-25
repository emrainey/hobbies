#pragma once

/// @file
/// The Vector Class

#include "xmmt/packs.hpp"

namespace xmmt {

/// An ordered set of values which can be operator-ed on in-bulk.
/// Classically, this is a vector and this is used as such.
/// @tparam pack_type The packing structure definition.
template <typename pack_type, size_t _dimensions>
class vector_ : public pack_type {
protected:
    /// Hold on to the number of elements needed
    constexpr static size_t dimensions = _dimensions;
    /// The data storage of the unit
    static_assert(std::is_floating_point<typename pack_type::element_type>::value,
                  "Must be either float, double or long double");
    static_assert(2 <= pack_type::number_of_elements, "Must be a Intel Parallel/Multiple Data data type");
    static_assert(dimensions <= pack_type::number_of_elements,
                  "The dimension of elements must fit into the number of parallel data slots");
    static_assert(2 <= dimensions and dimensions <= 4, "Must be larger than a single point but smaller than 5");

public:
    using element_type = typename pack_type::element_type;
    using parallel_type = typename pack_type::parallel_type;

    /// Default Constructor
    constexpr vector_() : pack_type{} {
    }

    /// Intrinsic Initializer
    constexpr vector_(parallel_type v) : pack_type{v} {
    }

    /// Forwarding Constructor. This allows the pack_type parameter constructors to be used.
    template <typename... Args>
    vector_(Args&&... args) : pack_type{std::forward<Args>(args)...} {
    }

    /// Copy Constructor
    vector_(vector_ const& other) : pack_type{other.data} {
    }

    /// Move Construction is just a Copy
    vector_(vector_&& other) : pack_type{other.data} {
    }

    /// Copy Assignment
    vector_& operator=(vector_ const& other) {
        pack_type::operator=(other.data);
        return (*this);
    }

    /// Move Assignment is Copy Assignment
    vector_& operator=(vector_&& other) {
        pack_type::operator=(std::move(other.data));
        return (*this);
    }

    /// Destructor
    ~vector_() = default;

    /// Initializer List Constructor
    constexpr vector_(std::initializer_list<element_type> list) : vector_{} {
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
    inline vector_& operator*=(element_type a) {
        if constexpr (pack_type::number_of_elements == 2) {
            simde__m128d tmp = simde_mm_setr_pd(a, a);
            pack_type::data = simde_mm_mul_pd(pack_type::data, tmp);
            static_cast<void>(tmp);
        } else {
            if constexpr (std::is_same_v<element_type, float>) {
                simde__m128 tmp = simde_mm_setr_ps(a, a, a, a);
                pack_type::data = simde_mm_mul_ps(pack_type::data, tmp);
                static_cast<void>(tmp);
            } else {
                simde__m256d tmp = simde_mm256_setr_pd(a, a, a, a);
                pack_type::data = simde_mm256_mul_pd(pack_type::data, tmp);
                static_cast<void>(tmp);
            }
        }
        return (*this);
    }

    /// Scales each value
    inline vector_& operator/=(element_type a) {
        if constexpr (pack_type::number_of_elements == 2) {
            pack_type::data = simde_mm_div_pd(pack_type::data, a.data);
        } else {
            if constexpr (std::is_same_v<element_type, float>) {
                pack_type::data = simde_mm_div_ps(pack_type::data, a.data);
            } else {
                pack_type::data = simde_mm256_div_pd(pack_type::data, a.data);
            }
        }
        return (*this);
    }

    /// Scales the vector by the approximate magnitude
    inline vector_& normalize() {
        double2 tmp;                 // we'll use this for union access only
        tmp.datum[0] = quadrance();  // put it in the bottom
        if constexpr (std::is_same_v<element_type, float>) {
            tmp.data = simde_mm_rsqrt_ps(tmp.data);  // get the inv sqrt approx
        } else {
            tmp.data = simde_mm_div_pd(simde_mm_set1_pd(1.0_p), simde_mm_sqrt_pd(tmp.data));
        }
        return operator*=(tmp.datum[0]);
    }

    /// Returns the sum of the squares of the elements
    inline element_type quadrance() {
        return dot((*this), (*this));
    }

    /// Returns the square root of the sum of squares of the elements
    inline element_type magnitude() {
        element_type q = quadrance();
        return sqrt(q);
    }

    /// Accumulate value
    inline vector_& operator+=(vector_ const& a) {
        if constexpr (pack_type::number_of_elements == 2) {
            pack_type::data = simde_mm_add_pd(pack_type::data, a.data);
        } else {
            if constexpr (std::is_same_v<element_type, float>) {
                pack_type::data = simde_mm_add_ps(pack_type::data, a.data);
            } else {
                pack_type::data = simde_mm256_add_pd(pack_type::data, a.data);
            }
        }
        return (*this);
    }

    inline vector_& operator-=(vector_ const& a) {
        if constexpr (pack_type::number_of_elements == 2) {
            pack_type::data = simde_mm_sub_pd(pack_type::data, a.data);
        } else {
            if constexpr (std::is_same_v<element_type, float>) {
                pack_type::data = simde_mm_sub_ps(pack_type::data, a.data);
            } else {
                pack_type::data = simde_mm256_sub_pd(pack_type::data, a.data);
            }
        }
        return (*this);
    }

    /// Equality Operator
    bool operator==(vector_ const& other) {
        bool ret = true;
        for (size_t n = 0; n < dimensions && ret; n++) {
            ret = (pack_type::datum[n] == other[n]);
        }
        return ret;
    }

    /// Inequality Operator
    bool operator!=(vector_ const& other) {
        return not operator==(other);
    }

    /// Unary Minus Operator (Inverts vector)
    inline vector_ operator-() const {
        vector_ c{};
        return (c - (*this));
    }

    void print(char const name[]) const {
        std::cout << name << " " << (*this) << std::endl;
    }

    /****** FRIENDS **************************************************************/

    friend inline vector_ operator+(vector_ const& a, vector_ const& b) {
        vector_ c{};
        if constexpr (pack_type::number_of_elements == 2) {
            c.data = simde_mm_add_pd(a.data, b.data);
        } else {
            if constexpr (std::is_same_v<element_type, float>) {
                c.data = simde_mm_add_ps(a.data, b.data);
            } else {
                c.data = simde_mm256_add_pd(a.data, b.data);
            }
        }
        return c;
    }

    friend inline vector_ operator-(vector_ const& a, vector_ const& b) {
        vector_ c{};
        if constexpr (pack_type::number_of_elements == 2) {
            c.data = simde_mm_sub_pd(a.data, b.data);
        } else {
            if constexpr (std::is_same_v<element_type, float>) {
                c.data = simde_mm_sub_ps(a.data, b.data);
            } else {
                c.data = simde_mm256_sub_pd(a.data, b.data);
            }
        }
        return c;
    }

    /// Pairwise/Hamard Scaling
    friend inline vector_ operator*(vector_ const& a, vector_ const& b) {
        vector_ c{};
        if constexpr (pack_type::number_of_elements == 2) {
            c.data = simde_mm_mul_pd(a.data, b.data);
        } else {
            if constexpr (std::is_same_v<element_type, float>) {
                c.data = simde_mm_mul_ps(a.data, b.data);
            } else {
                c.data = simde_mm256_mul_pd(a.data, b.data);
            }
        }
        return c;
    }

    friend element_type dot(vector_ const& a, vector_ const& b) {
        element_type d = 0.0_p;
        if constexpr (pack_type::number_of_elements == 2) {
            if constexpr (std::is_same_v<element_type, float>) {
                d = (a.datum[0] * b.datum[0]) + (a.datum[1] * b.datum[1]);
            } else {
                simde__m128d tmp = simde_mm_mul_pd(a.data, b.data);
                tmp = simde_mm_hadd_pd(tmp, tmp);
                vector_ c{tmp};
                d = c[0];
            }
        } else {  // 3 or 4 elements
            if constexpr (std::is_same_v<element_type, float>) {
                simde__m128 t = simde_mm_mul_ps(a.data, b.data);
                simde__m128 z = simde_mm_setzero_ps();
                simde__m128 h = simde_mm_hadd_ps(t, z);
                // t = simde_mm_permute_ps(h, 0xD8);
                h = simde_mm_hadd_ps(h, z);
                vector_ c{h};
                d = c[0];
            } else {
                simde__m256d t = simde_mm256_mul_pd(a.data, b.data);
                simde__m256d z = simde_mm256_setzero_pd();
                simde__m256d h = simde_mm256_hadd_pd(t, z);
                t = simde_mm256_permute4x64_pd(h, 0xD8);
                h = simde_mm256_hadd_pd(t, z);
                vector_ c{h};
                d = c[0];
            }
            if constexpr (pack_type::desired_elements == 3) {
                d -= 1.0_p;  // since element 3 should be 1.0_p
            }
        }
        return d;
    };

    friend vector_ cross(vector_ const& a, vector_ const& b) {
        static_assert(dimensions == 3, "Can't preform a cross product on anything but 3d");
        vector_ c{};
        if constexpr (dimensions == 3) {
            if constexpr (std::is_same_v<element_type, float>) {
                simde__m128 e = simde_mm_permute_ps(a.data, 0xC9);
                simde__m128 f = simde_mm_permute_ps(b.data, 0xC9);
                e = simde_mm_mul_ps(e, b.data);
                f = simde_mm_mul_ps(f, a.data);
                simde__m128 g = simde_mm_sub_ps(f, e);
                c.data = simde_mm_permute_ps(g, 0xC9);
                c[3] = 0.0_p;
            } else {
                simde__m256d e = simde_mm256_permute4x64_pd(a.data, 0xC9);
                simde__m256d f = simde_mm256_permute4x64_pd(b.data, 0xC9);
                e = simde_mm256_mul_pd(e, b.data);
                f = simde_mm256_mul_pd(f, a.data);
                simde__m256d g = simde_mm256_sub_pd(f, e);
                c.data = simde_mm256_permute4x64_pd(g, 0xC9);
                c[3] = 0.0_p;
            }
        }
        return c;
    }

    friend std::ostream& operator<<(std::ostream& os, vector_ const& a) {
        pack_type const& b = a;
        os << "vector " << b;
        return os;
    }
};

}  // namespace xmmt
