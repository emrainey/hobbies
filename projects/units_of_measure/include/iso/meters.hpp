///
/// @file
/// This is an auto-generated header for iso::meters.
/// A measure of distance
/// @note See README on re-generation.
/// @copyright Copyright 2024.
///
#pragma once
#include <cstdint>
#include <cstddef>

#include "iso/measurement.hpp"

/// iso comment
namespace iso {

/// A structure used to hold the postfix for meters
struct suffix_type_m {
    constexpr static const char *suffix = "m";
};

///
/// This is the iso::meters interface.
/// These have a default, generic, SI, copy, move, dtor and copy assign
/// And operator"" and equality and inequality
/// overload +=,-=,+,- with other units of the same type
/// overload *=,/=,*,/ with immediate values and atomics
/// overload ++,-- ?
/// overload >,<,==,!=,<=,>=
/// not overloaded %=,%
///
class meters : public measurement<precision, SI::prefix, suffix_type_m> {
public:
    /// Default constructor
    meters();
    /// Parameter constructor
    explicit meters(precision value);
    /// Copy Constructor
    meters(const meters& other);
    /// Move Constructor
    meters(meters&& other);
    /// Destructor
    ~meters() = default;
    /// Const Copy Assignment
    meters& operator=(const meters& other) noexcept;
    /// Copy Assignment
    meters& operator=(meters& other) noexcept;
    /// Move Assignment
    meters& operator=(meters&& other) noexcept;
    /// Equality Operator
    bool operator==(const meters& other) const;
    /// Inequality Operator
    bool operator!=(const meters& other) const;
    /// Accumulation Operator
    meters& operator+=(const meters& other);
    /// Deccumulation Operator
    meters& operator-=(const meters& other);
    /// Scaling Operator
    meters& operator*=(precision factor);
    /// Inverse Scaling Operator
    meters& operator/=(precision factor);
    /// Negating Operator
    meters operator-() const;
    /// Less Than Operator
    bool operator<(const meters& other) const;
    /// Less Than or Equal Operator
    bool operator<=(const meters& other) const;
    /// Greater Than Operator
    bool operator>(const meters& other) const;
    /// Greater Than or Equal Operator
    bool operator>=(const meters& other) const;
};

namespace operators {
    /// Addition operator
    meters operator+(const meters& A, const meters& B);
    /// Subtraction operator
    meters operator-(const meters& A, const meters& B);
    /// Left Scaling Operator
    meters operator*(const meters& A, precision factor);
    /// Right Scaling Operator
    meters operator*(precision factor, const meters& A);
    /// Inverse Scaling Operator
    meters operator/(const meters& A, precision factor);
}

namespace literals {
    /// Literal Double, Quote Operator
    meters operator""_m(long double value);
    /// Literal Int, Quote Operator
    meters operator""_m(unsigned long long value);
    /// Literal String, Quote Operator
    iso::meters operator""_m(const char a[]);
}

}