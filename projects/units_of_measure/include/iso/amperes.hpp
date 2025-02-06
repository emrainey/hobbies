///
/// @file
/// This is an auto-generated header for iso::amperes.
/// A measure of current in electricity
/// @note See README on re-generation.
/// @copyright Copyright 2025.
///
#pragma once
#include <cstdint>
#include <cstddef>

#include "iso/measurement.hpp"

/// iso comment
namespace iso {

/// A structure used to hold the postfix for amperes
struct suffix_type_A {
    constexpr static char const *suffix = "A";
};

///
/// This is the iso::amperes interface.
/// These have a default, generic, SI, copy, move, dtor and copy assign
/// And operator"" and equality and inequality
/// overload +=,-=,+,- with other units of the same type
/// overload *=,/=,*,/ with immediate values and atomics
/// overload ++,-- ?
/// overload >,<,==,!=,<=,>=
/// not overloaded %=,%
///
class amperes : public measurement<precision, SI::prefix, suffix_type_A> {
public:
    /// Default constructor
    amperes();
    /// Parameter constructor
    explicit amperes(precision value);
    /// Copy Constructor
    amperes(amperes const& other);
    /// Move Constructor
    amperes(amperes&& other);
    /// Destructor
    ~amperes() = default;
    /// Const Copy Assignment
    amperes& operator=(amperes const& other) noexcept;
    /// Copy Assignment
    amperes& operator=(amperes& other) noexcept;
    /// Move Assignment
    amperes& operator=(amperes&& other) noexcept;
    /// Equality Operator
    bool operator==(amperes const& other) const;
    /// Inequality Operator
    bool operator!=(amperes const& other) const;
    /// Accumulation Operator
    amperes& operator+=(amperes const& other);
    /// Deccumulation Operator
    amperes& operator-=(amperes const& other);
    /// Scaling Operator
    amperes& operator*=(precision factor);
    /// Inverse Scaling Operator
    amperes& operator/=(precision factor);
    /// Negating Operator
    amperes operator-() const;
    /// Less Than Operator
    bool operator<(amperes const& other) const;
    /// Less Than or Equal Operator
    bool operator<=(amperes const& other) const;
    /// Greater Than Operator
    bool operator>(amperes const& other) const;
    /// Greater Than or Equal Operator
    bool operator>=(amperes const& other) const;
};

namespace operators {
    /// Addition operator
    amperes operator+(amperes const& A, amperes const& B);
    /// Subtraction operator
    amperes operator-(amperes const& A, amperes const& B);
    /// Left Scaling Operator
    amperes operator*(amperes const& A, precision factor);
    /// Right Scaling Operator
    amperes operator*(precision factor, amperes const& A);
    /// Inverse Scaling Operator
    amperes operator/(amperes const& A, precision factor);
    /// Scale factor
    precision operator/(amperes const& A, amperes const& B);
}

namespace literals {
    /// Literal Double, Quote Operator
    amperes operator""_A(long double value);
    /// Literal Int, Quote Operator
    amperes operator""_A(unsigned long long value);
    /// Literal String, Quote Operator
    iso::amperes operator""_A(char const a[]);
}

}