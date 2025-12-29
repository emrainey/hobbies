///
/// @file
/// This is an auto-generated header for iso::ohms.
/// A measure of resistance to current
/// @note See README on re-generation.
/// @copyright Copyright 2025.
///
#pragma once
#include <cstdint>
#include <cstddef>

#include "iso/measurement.hpp"

/// iso comment
namespace iso {

/// A structure used to hold the postfix for ohms
struct suffix_type_Ohm {
    constexpr static char const* suffix = "Ohm";
};

///
/// This is the iso::ohms interface.
/// These have a default, generic, SI, copy, move, dtor and copy assign
/// And operator"" and equality and inequality
/// overload +=,-=,+,- with other units of the same type
/// overload *=,/=,*,/ with immediate values and atomics
/// overload ++,-- ?
/// overload >,<,==,!=,<=,>=
/// not overloaded %=,%
///
class ohms : public measurement<precision, SI::prefix, suffix_type_Ohm> {
public:
    /// Default constructor
    ohms();
    /// Parameter constructor
    explicit ohms(precision value);
    /// Copy Constructor
    ohms(ohms const& other);
    /// Move Constructor
    ohms(ohms&& other);
    /// Destructor
    ~ohms() = default;
    /// Const Copy Assignment
    ohms& operator=(ohms const& other) noexcept;
    /// Copy Assignment
    ohms& operator=(ohms& other) noexcept;
    /// Move Assignment
    ohms& operator=(ohms&& other) noexcept;
    /// Equality Operator
    bool operator==(ohms const& other) const;
    /// Inequality Operator
    bool operator!=(ohms const& other) const;
    /// Accumulation Operator
    ohms& operator+=(ohms const& other);
    /// Deccumulation Operator
    ohms& operator-=(ohms const& other);
    /// Scaling Operator
    ohms& operator*=(precision factor);
    /// Inverse Scaling Operator
    ohms& operator/=(precision factor);
    /// Negating Operator
    ohms operator-() const;
    /// Less Than Operator
    bool operator<(ohms const& other) const;
    /// Less Than or Equal Operator
    bool operator<=(ohms const& other) const;
    /// Greater Than Operator
    bool operator>(ohms const& other) const;
    /// Greater Than or Equal Operator
    bool operator>=(ohms const& other) const;
};

namespace operators {
/// Addition operator
ohms operator+(ohms const& A, ohms const& B);
/// Subtraction operator
ohms operator-(ohms const& A, ohms const& B);
/// Left Scaling Operator
ohms operator*(ohms const& A, precision factor);
/// Right Scaling Operator
ohms operator*(precision factor, ohms const& A);
/// Inverse Scaling Operator
ohms operator/(ohms const& A, precision factor);
/// Scale factor
precision operator/(ohms const& A, ohms const& B);
}  // namespace operators

namespace literals {
/// Literal Double, Quote Operator
ohms operator""_Ohm(long double value);
/// Literal Int, Quote Operator
ohms operator""_Ohm(unsigned long long value);
/// Literal String, Quote Operator
iso::ohms operator""_Ohm(char const a[]);
}  // namespace literals

}  // namespace iso