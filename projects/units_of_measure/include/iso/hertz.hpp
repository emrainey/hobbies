///
/// @file
/// This is an auto-generated header for iso::hertz.
/// A measure of frequency
/// @note See README on re-generation.
/// @copyright Copyright 2025.
///
#pragma once
#include <cstdint>
#include <cstddef>

#include "iso/measurement.hpp"

/// iso comment
namespace iso {

/// A structure used to hold the postfix for hertz
struct suffix_type_Hz {
    constexpr static char const* suffix = "Hz";
};

///
/// This is the iso::hertz interface.
/// These have a default, generic, SI, copy, move, dtor and copy assign
/// And operator"" and equality and inequality
/// overload +=,-=,+,- with other units of the same type
/// overload *=,/=,*,/ with immediate values and atomics
/// overload ++,-- ?
/// overload >,<,==,!=,<=,>=
/// not overloaded %=,%
///
class hertz : public measurement<precision, SI::prefix, suffix_type_Hz> {
public:
    /// Default constructor
    hertz();
    /// Parameter constructor
    explicit hertz(precision value);
    /// Copy Constructor
    hertz(hertz const& other);
    /// Move Constructor
    hertz(hertz&& other);
    /// Destructor
    ~hertz() = default;
    /// Const Copy Assignment
    hertz& operator=(hertz const& other) noexcept;
    /// Copy Assignment
    hertz& operator=(hertz& other) noexcept;
    /// Move Assignment
    hertz& operator=(hertz&& other) noexcept;
    /// Equality Operator
    bool operator==(hertz const& other) const;
    /// Inequality Operator
    bool operator!=(hertz const& other) const;
    /// Accumulation Operator
    hertz& operator+=(hertz const& other);
    /// Deccumulation Operator
    hertz& operator-=(hertz const& other);
    /// Scaling Operator
    hertz& operator*=(precision factor);
    /// Inverse Scaling Operator
    hertz& operator/=(precision factor);
    /// Negating Operator
    hertz operator-() const;
    /// Less Than Operator
    bool operator<(hertz const& other) const;
    /// Less Than or Equal Operator
    bool operator<=(hertz const& other) const;
    /// Greater Than Operator
    bool operator>(hertz const& other) const;
    /// Greater Than or Equal Operator
    bool operator>=(hertz const& other) const;
};

namespace operators {
/// Addition operator
hertz operator+(hertz const& A, hertz const& B);
/// Subtraction operator
hertz operator-(hertz const& A, hertz const& B);
/// Left Scaling Operator
hertz operator*(hertz const& A, precision factor);
/// Right Scaling Operator
hertz operator*(precision factor, hertz const& A);
/// Inverse Scaling Operator
hertz operator/(hertz const& A, precision factor);
/// Scale factor
precision operator/(hertz const& A, hertz const& B);
}  // namespace operators

namespace literals {
/// Literal Double, Quote Operator
hertz operator""_Hz(long double value);
/// Literal Int, Quote Operator
hertz operator""_Hz(unsigned long long value);
/// Literal String, Quote Operator
iso::hertz operator""_Hz(char const a[]);
}  // namespace literals

}  // namespace iso