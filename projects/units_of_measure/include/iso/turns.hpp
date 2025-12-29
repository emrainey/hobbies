///
/// @file
/// This is an auto-generated header for iso::turns.
/// A measure of an angle
/// @note See README on re-generation.
/// @copyright Copyright 2025.
///
#pragma once
#include <cstdint>
#include <cstddef>

#include "iso/measurement.hpp"

/// iso comment
namespace iso {

/// A structure used to hold the postfix for turns
struct suffix_type_Tau {
    constexpr static char const* suffix = "Tau";
};

///
/// This is the iso::turns interface.
/// These have a default, generic, SI, copy, move, dtor and copy assign
/// And operator"" and equality and inequality
/// overload +=,-=,+,- with other units of the same type
/// overload *=,/=,*,/ with immediate values and atomics
/// overload ++,-- ?
/// overload >,<,==,!=,<=,>=
/// not overloaded %=,%
///
class turns : public measurement<precision, SI::prefix, suffix_type_Tau> {
public:
    /// Default constructor
    turns();
    /// Parameter constructor
    explicit turns(precision value);
    /// Copy Constructor
    turns(turns const& other);
    /// Move Constructor
    turns(turns&& other);
    /// Destructor
    ~turns() = default;
    /// Const Copy Assignment
    turns& operator=(turns const& other) noexcept;
    /// Copy Assignment
    turns& operator=(turns& other) noexcept;
    /// Move Assignment
    turns& operator=(turns&& other) noexcept;
    /// Equality Operator
    bool operator==(turns const& other) const;
    /// Inequality Operator
    bool operator!=(turns const& other) const;
    /// Accumulation Operator
    turns& operator+=(turns const& other);
    /// Deccumulation Operator
    turns& operator-=(turns const& other);
    /// Scaling Operator
    turns& operator*=(precision factor);
    /// Inverse Scaling Operator
    turns& operator/=(precision factor);
    /// Negating Operator
    turns operator-() const;
    /// Less Than Operator
    bool operator<(turns const& other) const;
    /// Less Than or Equal Operator
    bool operator<=(turns const& other) const;
    /// Greater Than Operator
    bool operator>(turns const& other) const;
    /// Greater Than or Equal Operator
    bool operator>=(turns const& other) const;
};

namespace operators {
/// Addition operator
turns operator+(turns const& A, turns const& B);
/// Subtraction operator
turns operator-(turns const& A, turns const& B);
/// Left Scaling Operator
turns operator*(turns const& A, precision factor);
/// Right Scaling Operator
turns operator*(precision factor, turns const& A);
/// Inverse Scaling Operator
turns operator/(turns const& A, precision factor);
/// Scale factor
precision operator/(turns const& A, turns const& B);
}  // namespace operators

namespace literals {
/// Literal Double, Quote Operator
turns operator""_Tau(long double value);
/// Literal Int, Quote Operator
turns operator""_Tau(unsigned long long value);
/// Literal String, Quote Operator
iso::turns operator""_Tau(char const a[]);
}  // namespace literals

}  // namespace iso