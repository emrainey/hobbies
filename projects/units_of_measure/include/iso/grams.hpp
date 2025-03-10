///
/// @file
/// This is an auto-generated header for iso::grams.
/// A measurement of mass
/// @note See README on re-generation.
/// @copyright Copyright 2025.
///
#pragma once
#include <cstdint>
#include <cstddef>

#include "iso/measurement.hpp"

/// iso comment
namespace iso {

/// A structure used to hold the postfix for grams
struct suffix_type_g {
    constexpr static char const *suffix = "g";
};

///
/// This is the iso::grams interface.
/// These have a default, generic, SI, copy, move, dtor and copy assign
/// And operator"" and equality and inequality
/// overload +=,-=,+,- with other units of the same type
/// overload *=,/=,*,/ with immediate values and atomics
/// overload ++,-- ?
/// overload >,<,==,!=,<=,>=
/// not overloaded %=,%
///
class grams : public measurement<precision, SI::prefix, suffix_type_g> {
public:
    /// Default constructor
    grams();
    /// Parameter constructor
    explicit grams(precision value);
    /// Copy Constructor
    grams(grams const& other);
    /// Move Constructor
    grams(grams&& other);
    /// Destructor
    ~grams() = default;
    /// Const Copy Assignment
    grams& operator=(grams const& other) noexcept;
    /// Copy Assignment
    grams& operator=(grams& other) noexcept;
    /// Move Assignment
    grams& operator=(grams&& other) noexcept;
    /// Equality Operator
    bool operator==(grams const& other) const;
    /// Inequality Operator
    bool operator!=(grams const& other) const;
    /// Accumulation Operator
    grams& operator+=(grams const& other);
    /// Deccumulation Operator
    grams& operator-=(grams const& other);
    /// Scaling Operator
    grams& operator*=(precision factor);
    /// Inverse Scaling Operator
    grams& operator/=(precision factor);
    /// Negating Operator
    grams operator-() const;
    /// Less Than Operator
    bool operator<(grams const& other) const;
    /// Less Than or Equal Operator
    bool operator<=(grams const& other) const;
    /// Greater Than Operator
    bool operator>(grams const& other) const;
    /// Greater Than or Equal Operator
    bool operator>=(grams const& other) const;
};

namespace operators {
    /// Addition operator
    grams operator+(grams const& A, grams const& B);
    /// Subtraction operator
    grams operator-(grams const& A, grams const& B);
    /// Left Scaling Operator
    grams operator*(grams const& A, precision factor);
    /// Right Scaling Operator
    grams operator*(precision factor, grams const& A);
    /// Inverse Scaling Operator
    grams operator/(grams const& A, precision factor);
    /// Scale factor
    precision operator/(grams const& A, grams const& B);
}

namespace literals {
    /// Literal Double, Quote Operator
    grams operator""_g(long double value);
    /// Literal Int, Quote Operator
    grams operator""_g(unsigned long long value);
    /// Literal String, Quote Operator
    iso::grams operator""_g(char const a[]);
}

}