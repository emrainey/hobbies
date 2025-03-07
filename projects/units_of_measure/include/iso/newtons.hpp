///
/// @file
/// This is an auto-generated header for iso::newtons.
/// A measure of force
/// @note See README on re-generation.
/// @copyright Copyright 2025.
///
#pragma once
#include <cstdint>
#include <cstddef>

#include "iso/measurement.hpp"

/// iso comment
namespace iso {

/// A structure used to hold the postfix for newtons
struct suffix_type_N {
    constexpr static char const* suffix = "N";
};

///
/// This is the iso::newtons interface.
/// These have a default, generic, SI, copy, move, dtor and copy assign
/// And operator"" and equality and inequality
/// overload +=,-=,+,- with other units of the same type
/// overload *=,/=,*,/ with immediate values and atomics
/// overload ++,-- ?
/// overload >,<,==,!=,<=,>=
/// not overloaded %=,%
///
class newtons : public measurement<precision, SI::prefix, suffix_type_N> {
public:
    /// Default constructor
    newtons();
    /// Parameter constructor
    explicit newtons(precision value);
    /// Copy Constructor
    newtons(newtons const& other);
    /// Move Constructor
    newtons(newtons&& other);
    /// Destructor
    ~newtons() = default;
    /// Const Copy Assignment
    newtons& operator=(newtons const& other) noexcept;
    /// Copy Assignment
    newtons& operator=(newtons& other) noexcept;
    /// Move Assignment
    newtons& operator=(newtons&& other) noexcept;
    /// Equality Operator
    bool operator==(newtons const& other) const;
    /// Inequality Operator
    bool operator!=(newtons const& other) const;
    /// Accumulation Operator
    newtons& operator+=(newtons const& other);
    /// Deccumulation Operator
    newtons& operator-=(newtons const& other);
    /// Scaling Operator
    newtons& operator*=(precision factor);
    /// Inverse Scaling Operator
    newtons& operator/=(precision factor);
    /// Negating Operator
    newtons operator-() const;
    /// Less Than Operator
    bool operator<(newtons const& other) const;
    /// Less Than or Equal Operator
    bool operator<=(newtons const& other) const;
    /// Greater Than Operator
    bool operator>(newtons const& other) const;
    /// Greater Than or Equal Operator
    bool operator>=(newtons const& other) const;
};

namespace operators {
/// Addition operator
newtons operator+(newtons const& A, newtons const& B);
/// Subtraction operator
newtons operator-(newtons const& A, newtons const& B);
/// Left Scaling Operator
newtons operator*(newtons const& A, precision factor);
/// Right Scaling Operator
newtons operator*(precision factor, newtons const& A);
/// Inverse Scaling Operator
newtons operator/(newtons const& A, precision factor);
/// Scale factor
precision operator/(newtons const& A, newtons const& B);
}  // namespace operators

namespace literals {
/// Literal Double, Quote Operator
newtons operator""_N(long double value);
/// Literal Int, Quote Operator
newtons operator""_N(unsigned long long value);
/// Literal String, Quote Operator
iso::newtons operator""_N(char const a[]);
}  // namespace literals

}  // namespace iso