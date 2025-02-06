///
/// @file
/// This is an auto-generated header for iso::watts.
/// A measure of electrical power
/// @note See README on re-generation.
/// @copyright Copyright 2025.
///
#pragma once
#include <cstdint>
#include <cstddef>

#include "iso/measurement.hpp"

/// iso comment
namespace iso {

/// A structure used to hold the postfix for watts
struct suffix_type_W {
    constexpr static char const *suffix = "W";
};

///
/// This is the iso::watts interface.
/// These have a default, generic, SI, copy, move, dtor and copy assign
/// And operator"" and equality and inequality
/// overload +=,-=,+,- with other units of the same type
/// overload *=,/=,*,/ with immediate values and atomics
/// overload ++,-- ?
/// overload >,<,==,!=,<=,>=
/// not overloaded %=,%
///
class watts : public measurement<precision, SI::prefix, suffix_type_W> {
public:
    /// Default constructor
    watts();
    /// Parameter constructor
    explicit watts(precision value);
    /// Copy Constructor
    watts(watts const& other);
    /// Move Constructor
    watts(watts&& other);
    /// Destructor
    ~watts() = default;
    /// Const Copy Assignment
    watts& operator=(watts const& other) noexcept;
    /// Copy Assignment
    watts& operator=(watts& other) noexcept;
    /// Move Assignment
    watts& operator=(watts&& other) noexcept;
    /// Equality Operator
    bool operator==(watts const& other) const;
    /// Inequality Operator
    bool operator!=(watts const& other) const;
    /// Accumulation Operator
    watts& operator+=(watts const& other);
    /// Deccumulation Operator
    watts& operator-=(watts const& other);
    /// Scaling Operator
    watts& operator*=(precision factor);
    /// Inverse Scaling Operator
    watts& operator/=(precision factor);
    /// Negating Operator
    watts operator-() const;
    /// Less Than Operator
    bool operator<(watts const& other) const;
    /// Less Than or Equal Operator
    bool operator<=(watts const& other) const;
    /// Greater Than Operator
    bool operator>(watts const& other) const;
    /// Greater Than or Equal Operator
    bool operator>=(watts const& other) const;
};

namespace operators {
    /// Addition operator
    watts operator+(watts const& A, watts const& B);
    /// Subtraction operator
    watts operator-(watts const& A, watts const& B);
    /// Left Scaling Operator
    watts operator*(watts const& A, precision factor);
    /// Right Scaling Operator
    watts operator*(precision factor, watts const& A);
    /// Inverse Scaling Operator
    watts operator/(watts const& A, precision factor);
    /// Scale factor
    precision operator/(watts const& A, watts const& B);
}

namespace literals {
    /// Literal Double, Quote Operator
    watts operator""_W(long double value);
    /// Literal Int, Quote Operator
    watts operator""_W(unsigned long long value);
    /// Literal String, Quote Operator
    iso::watts operator""_W(char const a[]);
}

}