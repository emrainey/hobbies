///
/// @file
/// This is an auto-generated header for iso::meters.
/// A measure of distance
/// @note See README on re-generation.
/// @copyright Copyright 2025.
///
#pragma once
#include <cstdint>
#include <cstddef>

#include "iso/measurement.hpp"

/// iso comment
namespace iso {

/// A structure used to hold the postfix for meters
struct suffix_type_m {
    constexpr static char const *suffix = "m";
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
    meters(meters const& other);
    /// Move Constructor
    meters(meters&& other);
    /// Destructor
    ~meters() = default;
    /// Const Copy Assignment
    meters& operator=(meters const& other) noexcept;
    /// Copy Assignment
    meters& operator=(meters& other) noexcept;
    /// Move Assignment
    meters& operator=(meters&& other) noexcept;
    /// Equality Operator
    bool operator==(meters const& other) const;
    /// Inequality Operator
    bool operator!=(meters const& other) const;
    /// Accumulation Operator
    meters& operator+=(meters const& other);
    /// Deccumulation Operator
    meters& operator-=(meters const& other);
    /// Scaling Operator
    meters& operator*=(precision factor);
    /// Inverse Scaling Operator
    meters& operator/=(precision factor);
    /// Negating Operator
    meters operator-() const;
    /// Less Than Operator
    bool operator<(meters const& other) const;
    /// Less Than or Equal Operator
    bool operator<=(meters const& other) const;
    /// Greater Than Operator
    bool operator>(meters const& other) const;
    /// Greater Than or Equal Operator
    bool operator>=(meters const& other) const;
};

namespace operators {
    /// Addition operator
    meters operator+(meters const& A, meters const& B);
    /// Subtraction operator
    meters operator-(meters const& A, meters const& B);
    /// Left Scaling Operator
    meters operator*(meters const& A, precision factor);
    /// Right Scaling Operator
    meters operator*(precision factor, meters const& A);
    /// Inverse Scaling Operator
    meters operator/(meters const& A, precision factor);
    /// Scale factor
    precision operator/(meters const& A, meters const& B);
}

namespace literals {
    /// Literal Double, Quote Operator
    meters operator""_m(long double value);
    /// Literal Int, Quote Operator
    meters operator""_m(unsigned long long value);
    /// Literal String, Quote Operator
    iso::meters operator""_m(char const a[]);
}

}