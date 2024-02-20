///
/// @file
/// This is an auto-generated header for iso::joules.
/// A measure of energy
/// @note See README on re-generation.
/// @copyright Copyright 2024.
///
#pragma once
#include <cstdint>
#include <cstddef>

#include "iso/measurement.hpp"

/// iso comment
namespace iso {

/// A structure used to hold the postfix for joules
struct suffix_type_J {
    constexpr static char const *suffix = "J";
};

///
/// This is the iso::joules interface.
/// These have a default, generic, SI, copy, move, dtor and copy assign
/// And operator"" and equality and inequality
/// overload +=,-=,+,- with other units of the same type
/// overload *=,/=,*,/ with immediate values and atomics
/// overload ++,-- ?
/// overload >,<,==,!=,<=,>=
/// not overloaded %=,%
///
class joules : public measurement<precision, SI::prefix, suffix_type_J> {
public:
    /// Default constructor
    joules();
    /// Parameter constructor
    explicit joules(precision value);
    /// Copy Constructor
    joules(joules const& other);
    /// Move Constructor
    joules(joules&& other);
    /// Destructor
    ~joules() = default;
    /// Const Copy Assignment
    joules& operator=(joules const& other) noexcept;
    /// Copy Assignment
    joules& operator=(joules& other) noexcept;
    /// Move Assignment
    joules& operator=(joules&& other) noexcept;
    /// Equality Operator
    bool operator==(joules const& other) const;
    /// Inequality Operator
    bool operator!=(joules const& other) const;
    /// Accumulation Operator
    joules& operator+=(joules const& other);
    /// Deccumulation Operator
    joules& operator-=(joules const& other);
    /// Scaling Operator
    joules& operator*=(precision factor);
    /// Inverse Scaling Operator
    joules& operator/=(precision factor);
    /// Negating Operator
    joules operator-() const;
    /// Less Than Operator
    bool operator<(joules const& other) const;
    /// Less Than or Equal Operator
    bool operator<=(joules const& other) const;
    /// Greater Than Operator
    bool operator>(joules const& other) const;
    /// Greater Than or Equal Operator
    bool operator>=(joules const& other) const;
};

namespace operators {
    /// Addition operator
    joules operator+(joules const& A, joules const& B);
    /// Subtraction operator
    joules operator-(joules const& A, joules const& B);
    /// Left Scaling Operator
    joules operator*(joules const& A, precision factor);
    /// Right Scaling Operator
    joules operator*(precision factor, joules const& A);
    /// Inverse Scaling Operator
    joules operator/(joules const& A, precision factor);
}

namespace literals {
    /// Literal Double, Quote Operator
    joules operator""_J(long double value);
    /// Literal Int, Quote Operator
    joules operator""_J(unsigned long long value);
    /// Literal String, Quote Operator
    iso::joules operator""_J(char const a[]);
}

}