/**
 * @file
 * This is an auto-generated header for iso::seconds.
 * A measure of time
 * @note See README on re-generation.
 * @copyright Copyright 2022.
 */
#pragma once
#include <cstdint>
#include <cstddef>

#include "iso/measurement.hpp"

/** iso comment */
namespace iso {

/** A structure used to hold the postfix for seconds */
struct suffix_type_sec {
    constexpr static const char *suffix = "sec";
};

/**
 * This is the iso::seconds interface.
 * These have a default, generic, SI, copy, move, dtor and copy assign
 * And operator"" and equality and inequality
 * overload +=,-=,+,- with other units of the same type
 * overload *=,/=,*,/ with immediate values and atomics
 * overload ++,-- ?
 * overload >,<,==,!=,<=,>=
 * not overloaded %=,%
 */
class seconds : public measurement<double, SI::prefix, suffix_type_sec> {
public:
    /** Default constructor */
    seconds();
    /** Parameter constructor */
    explicit seconds(double value);
    /** Copy Constructor */
    seconds(const seconds& other);
    /** Move Constructor */
    seconds(seconds&& other);
    /** Destructor */
    ~seconds() = default;
    /** Const Copy Assignment */
    seconds& operator=(const seconds& other) noexcept;
    /** Copy Assignment */
    seconds& operator=(seconds& other) noexcept;
    /** Move Assignment */
    seconds& operator=(seconds&& other) noexcept;
    /** Equality Operator */
    bool operator==(const seconds& other) const;
    /** Inequality Operator */
    bool operator!=(const seconds& other) const;
    /** Accumulation Operator */
    seconds& operator+=(const seconds& other);
    /** Deccumulation Operator */
    seconds& operator-=(const seconds& other);
    /** Scaling Operator */
    seconds& operator*=(double factor);
    /** Inverse Scaling Operator */
    seconds& operator/=(double factor);
    /** Negating Operator */
    seconds operator-() const;
    /** Less Than Operator */
    bool operator<(const seconds& other) const;
    /** Less Than or Equal Operator */
    bool operator<=(const seconds& other) const;
    /** Greater Than Operator */
    bool operator>(const seconds& other) const;
    /** Greater Than or Equal Operator */
    bool operator>=(const seconds& other) const;
};

namespace operators {
    /** Addition operator */
    seconds operator+(const seconds& A, const seconds& B);
    /** Subtraction operator */
    seconds operator-(const seconds& A, const seconds& B);
    /** Left Scaling Operator */
    seconds operator*(const seconds& A, double factor);
    /** Right Scaling Operator */
    seconds operator*(double factor, const seconds& A);
    /** Inverse Scaling Operator */
    seconds operator/(const seconds& A, double factor);
}

namespace literals {
    /** Literal Double, Quote Operator */
    seconds operator""_sec(long double value);
    /** Literal Int, Quote Operator */
    seconds operator""_sec(unsigned long long value);
    /** Literal String, Quote Operator */
    iso::seconds operator""_sec(const char a[]);
}

}