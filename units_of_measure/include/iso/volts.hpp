/**
 * @file
 * This is an auto-generated header for iso::volts.
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

/** A structure used to hold the postfix for volts */
struct suffix_type_V {
    constexpr static const char* suffix = "V";
};

/**
 * This is the iso::volts interface.
 * These have a default, generic, SI, copy, move, dtor and copy assign
 * And operator"" and equality and inequality
 * overload +=,-=,+,- with other units of the same type
 * overload *=,/=,*,/ with immediate values and atomics
 * overload ++,-- ?
 * overload >,<,==,!=,<=,>=
 * not overloaded %=,%
 */
class volts : public measurement<double, SI::prefix, suffix_type_V> {
public:
    /** Default constructor */
    volts();
    /** Parameter constructor */
    explicit volts(double value);
    /** Copy Constructor */
    volts(const volts& other);
    /** Move Constructor */
    volts(volts&& other);
    /** Destructor */
    ~volts() = default;
    /** Const Copy Assignment */
    volts& operator=(const volts& other) noexcept;
    /** Copy Assignment */
    volts& operator=(volts& other) noexcept;
    /** Move Assignment */
    volts& operator=(volts&& other) noexcept;
    /** Equality Operator */
    bool operator==(const volts& other) const;
    /** Inequality Operator */
    bool operator!=(const volts& other) const;
    /** Accumulation Operator */
    volts& operator+=(const volts& other);
    /** Deccumulation Operator */
    volts& operator-=(const volts& other);
    /** Scaling Operator */
    volts& operator*=(double factor);
    /** Inverse Scaling Operator */
    volts& operator/=(double factor);
    /** Negating Operator */
    volts operator-() const;
    /** Less Than Operator */
    bool operator<(const volts& other) const;
    /** Less Than or Equal Operator */
    bool operator<=(const volts& other) const;
    /** Greater Than Operator */
    bool operator>(const volts& other) const;
    /** Greater Than or Equal Operator */
    bool operator>=(const volts& other) const;
};

namespace operators {
/** Addition operator */
volts operator+(const volts& A, const volts& B);
/** Subtraction operator */
volts operator-(const volts& A, const volts& B);
/** Left Scaling Operator */
volts operator*(const volts& A, double factor);
/** Right Scaling Operator */
volts operator*(double factor, const volts& A);
/** Inverse Scaling Operator */
volts operator/(const volts& A, double factor);
}  // namespace operators

namespace literals {
/** Literal Double, Quote Operator */
volts operator""_V(long double value);
/** Literal Int, Quote Operator */
volts operator""_V(unsigned long long value);
/** Literal String, Quote Operator */
iso::volts operator""_V(const char a[]);
}  // namespace literals

}  // namespace iso