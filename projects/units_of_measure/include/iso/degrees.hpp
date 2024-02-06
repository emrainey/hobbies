///
/// @file
/// This is an auto-generated header for iso::degrees.
/// A measure of an angle
/// @note See README on re-generation.
/// @copyright Copyright 2024.
///
#pragma once
#include <cstdint>
#include <cstddef>

#include "iso/measurement.hpp"

/// iso comment
namespace iso {

/// A structure used to hold the postfix for degrees
struct suffix_type_d {
    constexpr static const char *suffix = "d";
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
    degrees(const degrees& other);
    /// Move Constructor
    degrees(degrees&& other);
    /// Destructor
    ~degrees() = default;
    /// Const Copy Assignment
    degrees& operator=(const degrees& other) noexcept;
    /// Copy Assignment
    degrees& operator=(degrees& other) noexcept;
    /// Move Assignment
    degrees& operator=(degrees&& other) noexcept;
    /// Equality Operator
    bool operator==(const degrees& other) const;
    /// Inequality Operator
    bool operator!=(const degrees& other) const;
    /// Accumulation Operator
    degrees& operator+=(const degrees& other);
    /// Deccumulation Operator
    degrees& operator-=(const degrees& other);
    /// Scaling Operator
    degrees& operator*=(precision factor);
    /// Inverse Scaling Operator
    degrees& operator/=(precision factor);
    /// Negating Operator
    degrees operator-() const;
    /// Less Than Operator
    bool operator<(const degrees& other) const;
    /// Less Than or Equal Operator
    bool operator<=(const degrees& other) const;
    /// Greater Than Operator
    bool operator>(const degrees& other) const;
    /// Greater Than or Equal Operator
    bool operator>=(const degrees& other) const;
};

namespace operators {
    /// Addition operator
    degrees operator+(const degrees& A, const degrees& B);
    /// Subtraction operator
    degrees operator-(const degrees& A, const degrees& B);
    /// Left Scaling Operator
    degrees operator*(const degrees& A, precision factor);
    /// Right Scaling Operator
    degrees operator*(precision factor, const degrees& A);
    /// Inverse Scaling Operator
    degrees operator/(const degrees& A, precision factor);
}

namespace literals {
    /// Literal Double, Quote Operator
    degrees operator""_d(long double value);
    /// Literal Int, Quote Operator
    degrees operator""_d(unsigned long long value);
    /// Literal String, Quote Operator
    iso::degrees operator""_d(const char a[]);
}

}