///
/// @file
/// This is an auto-generated header for iso::volts.
/// A measure of the electrical potential
/// @note See README on re-generation.
/// @copyright Copyright 2025.
///
#pragma once
#include <cstdint>
#include <cstddef>

#include "iso/measurement.hpp"

/// iso comment
namespace iso {

/// A structure used to hold the postfix for volts
struct suffix_type_V {
    constexpr static char const* suffix = "V";
};

///
/// This is the iso::volts interface.
/// These have a default, generic, SI, copy, move, dtor and copy assign
/// And operator"" and equality and inequality
/// overload +=,-=,+,- with other units of the same type
/// overload *=,/=,*,/ with immediate values and atomics
/// overload ++,-- ?
/// overload >,<,==,!=,<=,>=
/// not overloaded %=,%
///
class volts : public measurement<precision, SI::prefix, suffix_type_V> {
public:
    /// Default constructor
    volts();
    /// Parameter constructor
    explicit volts(precision value);
    /// Copy Constructor
    volts(volts const& other);
    /// Move Constructor
    volts(volts&& other);
    /// Destructor
    ~volts() = default;
    /// Const Copy Assignment
    volts& operator=(volts const& other) noexcept;
    /// Copy Assignment
    volts& operator=(volts& other) noexcept;
    /// Move Assignment
    volts& operator=(volts&& other) noexcept;
    /// Equality Operator
    bool operator==(volts const& other) const;
    /// Inequality Operator
    bool operator!=(volts const& other) const;
    /// Accumulation Operator
    volts& operator+=(volts const& other);
    /// Deccumulation Operator
    volts& operator-=(volts const& other);
    /// Scaling Operator
    volts& operator*=(precision factor);
    /// Inverse Scaling Operator
    volts& operator/=(precision factor);
    /// Negating Operator
    volts operator-() const;
    /// Less Than Operator
    bool operator<(volts const& other) const;
    /// Less Than or Equal Operator
    bool operator<=(volts const& other) const;
    /// Greater Than Operator
    bool operator>(volts const& other) const;
    /// Greater Than or Equal Operator
    bool operator>=(volts const& other) const;
};

namespace operators {
/// Addition operator
volts operator+(volts const& A, volts const& B);
/// Subtraction operator
volts operator-(volts const& A, volts const& B);
/// Left Scaling Operator
volts operator*(volts const& A, precision factor);
/// Right Scaling Operator
volts operator*(precision factor, volts const& A);
/// Inverse Scaling Operator
volts operator/(volts const& A, precision factor);
/// Scale factor
precision operator/(volts const& A, volts const& B);
}  // namespace operators

namespace literals {
/// Literal Double, Quote Operator
volts operator""_V(long double value);
/// Literal Int, Quote Operator
volts operator""_V(unsigned long long value);
/// Literal String, Quote Operator
iso::volts operator""_V(char const a[]);
}  // namespace literals

}  // namespace iso