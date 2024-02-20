///
/// @file
/// This is an auto-generated header for iso::seconds.
/// A measure of time
/// @note See README on re-generation.
/// @copyright Copyright 2024.
///
#pragma once
#include <cstdint>
#include <cstddef>

#include "iso/measurement.hpp"

/// iso comment
namespace iso {

/// A structure used to hold the postfix for seconds
struct suffix_type_sec {
    constexpr static char const *suffix = "sec";
};

///
/// This is the iso::seconds interface.
/// These have a default, generic, SI, copy, move, dtor and copy assign
/// And operator"" and equality and inequality
/// overload +=,-=,+,- with other units of the same type
/// overload *=,/=,*,/ with immediate values and atomics
/// overload ++,-- ?
/// overload >,<,==,!=,<=,>=
/// not overloaded %=,%
///
class seconds : public measurement<precision, SI::prefix, suffix_type_sec> {
public:
    /// Default constructor
    seconds();
    /// Parameter constructor
    explicit seconds(precision value);
    /// Copy Constructor
    seconds(seconds const& other);
    /// Move Constructor
    seconds(seconds&& other);
    /// Destructor
    ~seconds() = default;
    /// Const Copy Assignment
    seconds& operator=(seconds const& other) noexcept;
    /// Copy Assignment
    seconds& operator=(seconds& other) noexcept;
    /// Move Assignment
    seconds& operator=(seconds&& other) noexcept;
    /// Equality Operator
    bool operator==(seconds const& other) const;
    /// Inequality Operator
    bool operator!=(seconds const& other) const;
    /// Accumulation Operator
    seconds& operator+=(seconds const& other);
    /// Deccumulation Operator
    seconds& operator-=(seconds const& other);
    /// Scaling Operator
    seconds& operator*=(precision factor);
    /// Inverse Scaling Operator
    seconds& operator/=(precision factor);
    /// Negating Operator
    seconds operator-() const;
    /// Less Than Operator
    bool operator<(seconds const& other) const;
    /// Less Than or Equal Operator
    bool operator<=(seconds const& other) const;
    /// Greater Than Operator
    bool operator>(seconds const& other) const;
    /// Greater Than or Equal Operator
    bool operator>=(seconds const& other) const;
};

namespace operators {
    /// Addition operator
    seconds operator+(seconds const& A, seconds const& B);
    /// Subtraction operator
    seconds operator-(seconds const& A, seconds const& B);
    /// Left Scaling Operator
    seconds operator*(seconds const& A, precision factor);
    /// Right Scaling Operator
    seconds operator*(precision factor, seconds const& A);
    /// Inverse Scaling Operator
    seconds operator/(seconds const& A, precision factor);
}

namespace literals {
    /// Literal Double, Quote Operator
    seconds operator""_sec(long double value);
    /// Literal Int, Quote Operator
    seconds operator""_sec(unsigned long long value);
    /// Literal String, Quote Operator
    iso::seconds operator""_sec(char const a[]);
}

}