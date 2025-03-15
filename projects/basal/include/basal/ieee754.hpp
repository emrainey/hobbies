#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include <cinttypes>
#include <type_traits>

namespace basal {

#if defined(USE_PRECISION_AS_FLOAT)
using precision = float;
#define PRIprecision "f"
#define SCNprecision "f"
constexpr bool use_high_precision = false;
#else
using precision = double;
#define PRIprecision "lf"
#define SCNprecision "lf"
constexpr bool use_high_precision = true;
#endif

static_assert(std::numeric_limits<precision>::is_iec559, "Must conform to IEEE754");

namespace literals {
/// When basal's type for precision is defined you can use 1.0_p and it will transparently go to the right type
constexpr inline precision operator""_p(long double v) {
    return precision(v);
}
static_assert(std::is_same<decltype(1.0_p), basal::precision>(), "Must be a precision type");
static_assert(std::is_floating_point<decltype(1.0_p)>(), "Must be a floating value");
}  // namespace literals

/// If high precision is used, it's 1/(1024*1024) else only (1/1024)
constexpr static precision epsilon = use_high_precision ? 0x1.0p-20 : 0x1.0p-10;
// 1/(1024) == 0.000007875504032
// 1/(1024*1024) == 0.000000953674316

/// A small value for use in comparisons.
/// When high precision is used, it's (1/1024) else only (1/256)
constexpr static precision smallish = use_high_precision ? 0x1.0p-10 : 0x1.0p-8;

/// Holds a version of a Quiet (non signalling) NaN for comparisons
constexpr static precision nan = std::numeric_limits<precision>::quiet_NaN();
constexpr static precision pos_inf = std::numeric_limits<precision>::infinity();
constexpr static precision neg_inf = std::numeric_limits<precision>::infinity() * -1;
constexpr static precision pos_zero = precision(0.0);
constexpr static precision neg_zero = precision(-0.0);
constexpr static precision sqrt_2 = precision(1.414213562373095);      // sqrt(2.0);
constexpr static precision sqrt_3 = precision(1.732050807568877);      // sqrt(3.0);
constexpr static precision inv_sqrt_3 = precision(0.577350269189626);  // 1.0/sqrt(3.0);

inline precision clamp(precision min, precision value, precision max) {
    return std::max(min, std::min(value, max));
}

/// Returns true if the two precisions are within the range of each other.
inline bool nearly_equals(precision a, precision b, precision range) {
    return (std::abs(a - b) <= std::abs(range));
}

/// Allows two precisions to be compared for equality.
inline bool nearly_equals(precision a, precision b) {
    return nearly_equals(a, b, basal::epsilon);
}

/// Determines if a precision is very close to zero
inline bool nearly_zero(precision a) {
    using namespace basal::literals;
    // return nearly_equals(a, 0.0_p, basal::epsilon);
    return std::abs(a) < basal::epsilon;
}

/// @brief Returns true if the precision is either exactly +0.0 or -0.0
/// @param a The precision to check
inline bool is_exactly_zero(precision a) {
    return (FP_ZERO == std::fpclassify(a));
}

/// @brief Returns true if the value is greater than or equal to zero (+/-0.0_p)
inline bool is_greater_than_or_equal_to_zero(precision a) {
    using namespace basal::literals;
    return (a > 0.0_p or is_exactly_zero(a));
}

/// @brief Returns true if the value is less than or equal to zero (+/-0.0_p)
inline bool is_less_than_or_equal_to_zero(precision a) {
    using namespace basal::literals;
    return (a < 0.0_p or is_exactly_zero(a));
}

/// A comparison which neither value is larger than the other, effectively equivalent
inline bool equivalent(precision const a, precision const b) {
    return not(a > b) and not(a < b);
}

/// Returns a random value between -1.0 and 1.0
inline precision rand_range(precision min_v, precision max_v) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<precision> dis(min_v, max_v);
    return dis(gen);
}

inline bool is_nan(precision a) {
    return (a != a);  // Only NaN is not equal to itself
}

/// @brief A structure for examining the IEEE754 representation of a single precision floating point number
struct single {
    constexpr single() : native{0.0f} {
    }
    constexpr single(float f) : native{f} {
    }

    static constexpr int bias = 127;
    static constexpr int min_exponent = -126;
    static constexpr int max_exponent = 127;
    static constexpr int special_exponent = 0b1111'1111;
    static constexpr int zero_exponent = 0b0000'0000;
    struct fields {
        std::uint32_t mantissa : 23;  // bits 0-22
        std::uint32_t exponent : 8;   // bits 23-30
        std::uint32_t sign : 1;       // bit 31
    };
    union {
        fields bits;
        std::uint32_t raw;
        float native;
    };

    friend std::ostream& operator<<(std::ostream& os, single const& v) {
        return os << "single=" << std::hex << "s:" << v.bits.sign << " e:(hex)" << v.bits.exponent << " (dec)"
                  << std::dec << v.bits.exponent << std::hex << " m:" << v.bits.mantissa << " float=" << v.native;
    }
};

/// @brief A structure for examining the IEEE754 representation of a half precision floating point number (float 16)
struct half {
    static constexpr int bias = 15;
    static constexpr int min_exponent = -14;
    static constexpr int max_exponent = 15;
    static constexpr int special_exponent = 0b11111;
    static constexpr int zero_exponent = 0b00000;
    constexpr half() : raw{0} {
    }
    constexpr half(float f) : raw{0} {
        from(f);
    }
    constexpr half(std::uint32_t s, std::uint32_t e, std::uint32_t m) : bits(m, e, s) {
    }

    half& operator=(float f) {
        from(f);
        return *this;
    }

    struct fields {
        constexpr fields(std::uint32_t m, std::uint32_t e, std::uint32_t s)
            : mantissa(m & 0x03FFU), exponent(e & 0x1FU), sign(s & 0x1U) {
        }
        std::uint16_t mantissa : 10;  // bits 0-9
        std::uint16_t exponent : 5;   // bits 10-14
        std::uint16_t sign : 1;       // bit 15
    };
    union {
        fields bits;
        std::uint16_t raw;
    };

    friend std::ostream& operator<<(std::ostream& os, half const& h) {
        return os << "half=" << std::hex << "s:" << h.bits.sign << " e:(hex)" << h.bits.exponent << " (dec)" << std::dec
                  << h.bits.exponent << std::hex << " m:" << h.bits.mantissa;
    }

    bool operator==(half const& h) const {
        bool same_sign = (bits.sign == h.bits.sign);
        bool same_exponent = (bits.exponent == h.bits.exponent);
        bool same_mantissa = (bits.mantissa == h.bits.mantissa);
        bool our_some_mantissa = (bits.mantissa > 0);
        bool their_some_mantissa = (h.bits.mantissa > 0);
        bool special_exp = (bits.exponent == special_exponent);
        bool zero_mantissa = (bits.mantissa == 0);
        bool zero = (bits.exponent == 0b0 and zero_mantissa);
        return ((zero and same_exponent and same_mantissa)  // zeros (+/-) are the same
                or (special_exp and same_sign and same_exponent and same_mantissa
                    and zero_mantissa)  // inf (+/-) not the same
                or (special_exp and same_exponent and our_some_mantissa and their_some_mantissa)  // nan
                or (same_sign and same_exponent and same_mantissa));
    }
    bool operator!=(half const& h) const {
        return not operator==(h);
    }

    bool is_exactly(half const& h) const {
        bool same_sign = (bits.sign == h.bits.sign);
        bool same_exponent = (bits.exponent == h.bits.exponent);
        bool same_mantissa = (bits.mantissa == h.bits.mantissa);
        return same_sign and same_exponent and same_mantissa;
    }

    bool is_nan() const {
        bool special_exp = (bits.exponent == special_exponent);
        bool some_mantissa = (bits.mantissa > 0);
        return special_exp and some_mantissa;  // sign is irrelevant
    }

    bool is_inf() const {
        bool special_exp = (bits.exponent == special_exponent);
        bool zero_mantissa = (bits.mantissa == 0);
        return special_exp and zero_mantissa;  // sign is irrelevant
    }

    bool is_zero() const {
        bool zero_mantissa = (bits.mantissa == 0);
        bool zero = (bits.exponent == 0b0 and zero_mantissa);
        return zero;  // sign is irrelevant
    }

    explicit operator float() const {
        single single;
        if (bits.exponent == special_exponent) {
            single.bits.exponent = single.special_exponent;
        } else if (bits.exponent == zero_exponent and bits.mantissa == 0) {
            single.bits.exponent = single.zero_exponent;
        } else {
            single.bits.exponent = (bits.exponent - bias) + single.bias;
        }
        single.bits.mantissa = static_cast<uint32_t>(bits.mantissa << 13U);
        single.bits.sign = bits.sign;
        return single.native;
    }

    void from(float f) {
        single single;
        single.native = f;
        // zero centered exponent
        std::int32_t e = static_cast<std::int32_t>(single.bits.exponent) - single.bias;
        if (single.bits.exponent == single.special_exponent) {
            // some form of inf or nan
            bits.sign = single.bits.sign;
            bits.exponent = special_exponent;
            bits.mantissa = single.bits.mantissa >> 13;
        } else if (single.bits.exponent == single.zero_exponent) {
            bits.sign = single.bits.sign;
            bits.exponent = zero_exponent;
            bits.mantissa = 0;
        } else if (e > max_exponent) {
            // + infinity
            bits.sign = 0b0;
            bits.exponent = special_exponent;
            bits.mantissa = 0b0;
        } else if (e < min_exponent) {
            // - infinity
            bits.sign = 0b1;
            bits.exponent = special_exponent;
            bits.mantissa = 0b0;
        } else {
            bits.sign = single.bits.sign;
            bits.exponent = static_cast<uint16_t>(e + bias);
            bits.mantissa = single.bits.mantissa >> 13;  // loose some precision!
        }
    }
};

namespace half_literals {
constexpr half operator""_hf(long double v) {
    float f = static_cast<float>(v);
    return half{f};
}
}  // namespace half_literals

namespace half_constants {
static const half positive_infinity{0b0, 0b1'1111, 0b0};
static const half negative_infinity{0b1, 0b1'1111, 0b0};
static const half positive_nan{0b0, 0b1'1111, 0b10'0000'0000};
static const half negative_nan{0b1, 0b1'1111, 0b11'1111'1111};
static const half positive_zero(0b0, 0b0, 0b0);
static const half negative_zero(0b1, 0b0, 0b0);
static const half epsilon{0b0, 0b0'0100, 0b11'1111'1111};
}  // namespace half_constants

}  // namespace basal