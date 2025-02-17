///
/// @file
/// This is an auto-generated header for iso::degrees.
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

/// A structure used to hold the postfix for degrees
struct suffix_type_d {
    constexpr static char const *suffix = "d";
};

///
/// This is the iso::degrees interface.
/// These have a default, generic, SI, copy, move, dtor and copy assign
/// And operator"" and equality and inequality
/// overload +=,-=,+,- with other units of the same type
/// overload *=,/=,*,/ with immediate values and atomics
/// overload ++,-- ?
/// overload >,<,==,!=,<=,>=
/// not overloaded %=,%
///
class degrees : public measurement<precision, SI::prefix, suffix_type_d> {
public:
    /// Default constructor
    degrees();
    /// Parameter constructor
    explicit degrees(precision value);
    /// Copy Constructor
    degrees(degrees const& other);
    /// Move Constructor
    degrees(degrees&& other);
    /// Destructor
    ~degrees() = default;
    /// Const Copy Assignment
    degrees& operator=(degrees const& other) noexcept;
    /// Copy Assignment
    degrees& operator=(degrees& other) noexcept;
    /// Move Assignment
    degrees& operator=(degrees&& other) noexcept;
    /// Equality Operator
    bool operator==(degrees const& other) const;
    /// Inequality Operator
    bool operator!=(degrees const& other) const;
    /// Accumulation Operator
    degrees& operator+=(degrees const& other);
    /// Deccumulation Operator
    degrees& operator-=(degrees const& other);
    /// Scaling Operator
    degrees& operator*=(precision factor);
    /// Inverse Scaling Operator
    degrees& operator/=(precision factor);
    /// Negating Operator
    degrees operator-() const;
    /// Less Than Operator
    bool operator<(degrees const& other) const;
    /// Less Than or Equal Operator
    bool operator<=(degrees const& other) const;
    /// Greater Than Operator
    bool operator>(degrees const& other) const;
    /// Greater Than or Equal Operator
    bool operator>=(degrees const& other) const;
};

namespace operators {
    /// Addition operator
    degrees operator+(degrees const& A, degrees const& B);
    /// Subtraction operator
    degrees operator-(degrees const& A, degrees const& B);
    /// Left Scaling Operator
    degrees operator*(degrees const& A, precision factor);
    /// Right Scaling Operator
    degrees operator*(precision factor, degrees const& A);
    /// Inverse Scaling Operator
    degrees operator/(degrees const& A, precision factor);
    /// Scale factor
    precision operator/(degrees const& A, degrees const& B);
}

namespace literals {
    /// Literal Double, Quote Operator
    degrees operator""_deg(long double value);
    /// Literal Int, Quote Operator
    degrees operator""_deg(unsigned long long value);
    /// Literal String, Quote Operator
    iso::degrees operator""_deg(char const a[]);
}

}