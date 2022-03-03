/**
 * @file
 * This is an auto-generated header for iso::pascals.
 *
 * @note See README on re-generation.
 * @copyright Copyright 2022.
 */
#pragma once
#include <cstddef>
#include <cstdint>

#include "iso/measurement.hpp"

/** iso comment */
namespace iso {

/** A structure used to hold the postfix for pascals */
struct suffix_type_Pa {
    constexpr static const char* suffix = "Pa";
};

/**
 * This is the iso::pascals interface.
 * These have a default, generic, SI, copy, move, dtor and copy assign
 * And operator"" and equality and inequality
 * overload +=,-=,+,- with other units of the same type
 * overload *=,/=,*,/ with immediate values and atomics
 * overload ++,-- ?
 * overload >,<,==,!=,<=,>=
 * not overloaded %=,%
 */
class pascals : public measurement<double, SI::prefix, suffix_type_Pa> {
public:
    /** Default constructor */
    pascals();
    /** Parameter constructor */
    explicit pascals(double value);
    /** Copy Constructor */
    pascals(const pascals& other);
    /** Move Constructor */
    pascals(pascals&& other);
    /** Destructor */
    ~pascals() = default;
    /** Const Copy Assignment */
    pascals& operator=(const pascals& other) noexcept;
    /** Copy Assignment */
    pascals& operator=(pascals& other) noexcept;
    /** Move Assignment */
    pascals& operator=(pascals&& other) noexcept;
    /** Equality Operator */
    bool operator==(const pascals& other) const;
    /** Inequality Operator */
    bool operator!=(const pascals& other) const;
    /** Accumulation Operator */
    pascals& operator+=(const pascals& other);
    /** Deccumulation Operator */
    pascals& operator-=(const pascals& other);
    /** Scaling Operator */
    pascals& operator*=(double factor);
    /** Inverse Scaling Operator */
    pascals& operator/=(double factor);
    /** Negating Operator */
    pascals operator-() const;
    /** Less Than Operator */
    bool operator<(const pascals& other) const;
    /** Less Than or Equal Operator */
    bool operator<=(const pascals& other) const;
    /** Greater Than Operator */
    bool operator>(const pascals& other) const;
    /** Greater Than or Equal Operator */
    bool operator>=(const pascals& other) const;
};

namespace operators {
/** Addition operator */
pascals operator+(const pascals& A, const pascals& B);
/** Subtraction operator */
pascals operator-(const pascals& A, const pascals& B);
/** Left Scaling Operator */
pascals operator*(const pascals& A, double factor);
/** Right Scaling Operator */
pascals operator*(double factor, const pascals& A);
/** Inverse Scaling Operator */
pascals operator/(const pascals& A, double factor);
}  // namespace operators

namespace literals {
/** Literal Double, Quote Operator */
pascals operator""_Pa(long double value);
/** Literal Int, Quote Operator */
pascals operator""_Pa(unsigned long long value);
/** Literal String, Quote Operator */
iso::pascals operator""_Pa(const char a[]);
}  // namespace literals

}  // namespace iso