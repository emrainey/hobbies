///
/// @file
/// This is an auto-generated header for iso::pascals.
/// A measure of pressure
/// @note See README on re-generation.
/// @copyright Copyright 2025.
///
#pragma once
#include <cstdint>
#include <cstddef>

#include "iso/measurement.hpp"

/// iso comment
namespace iso {

/// A structure used to hold the postfix for pascals
struct suffix_type_Pa {
    constexpr static char const *suffix = "Pa";
};

///
/// This is the iso::pascals interface.
/// These have a default, generic, SI, copy, move, dtor and copy assign
/// And operator"" and equality and inequality
/// overload +=,-=,+,- with other units of the same type
/// overload *=,/=,*,/ with immediate values and atomics
/// overload ++,-- ?
/// overload >,<,==,!=,<=,>=
/// not overloaded %=,%
///
class pascals : public measurement<precision, SI::prefix, suffix_type_Pa> {
public:
    /// Default constructor
    pascals();
    /// Parameter constructor
    explicit pascals(precision value);
    /// Copy Constructor
    pascals(pascals const& other);
    /// Move Constructor
    pascals(pascals&& other);
    /// Destructor
    ~pascals() = default;
    /// Const Copy Assignment
    pascals& operator=(pascals const& other) noexcept;
    /// Copy Assignment
    pascals& operator=(pascals& other) noexcept;
    /// Move Assignment
    pascals& operator=(pascals&& other) noexcept;
    /// Equality Operator
    bool operator==(pascals const& other) const;
    /// Inequality Operator
    bool operator!=(pascals const& other) const;
    /// Accumulation Operator
    pascals& operator+=(pascals const& other);
    /// Deccumulation Operator
    pascals& operator-=(pascals const& other);
    /// Scaling Operator
    pascals& operator*=(precision factor);
    /// Inverse Scaling Operator
    pascals& operator/=(precision factor);
    /// Negating Operator
    pascals operator-() const;
    /// Less Than Operator
    bool operator<(pascals const& other) const;
    /// Less Than or Equal Operator
    bool operator<=(pascals const& other) const;
    /// Greater Than Operator
    bool operator>(pascals const& other) const;
    /// Greater Than or Equal Operator
    bool operator>=(pascals const& other) const;
};

namespace operators {
    /// Addition operator
    pascals operator+(pascals const& A, pascals const& B);
    /// Subtraction operator
    pascals operator-(pascals const& A, pascals const& B);
    /// Left Scaling Operator
    pascals operator*(pascals const& A, precision factor);
    /// Right Scaling Operator
    pascals operator*(precision factor, pascals const& A);
    /// Inverse Scaling Operator
    pascals operator/(pascals const& A, precision factor);
    /// Scale factor
    precision operator/(pascals const& A, pascals const& B);
}

namespace literals {
    /// Literal Double, Quote Operator
    pascals operator""_Pa(long double value);
    /// Literal Int, Quote Operator
    pascals operator""_Pa(unsigned long long value);
    /// Literal String, Quote Operator
    iso::pascals operator""_Pa(char const a[]);
}

}