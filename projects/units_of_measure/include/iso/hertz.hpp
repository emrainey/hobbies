/**
 * @file
 * This is an auto-generated header for iso::hertz.
 * A measure of frequency
 * @note See README on re-generation.
 * @copyright Copyright 2022.
 */
#pragma once
#include <cstdint>
#include <cstddef>

#include "iso/measurement.hpp"

/** iso comment */
namespace iso {

/** A structure used to hold the postfix for hertz */
struct suffix_type_Hz {
    constexpr static const char *suffix = "Hz";
};

/**
 * This is the iso::hertz interface.
 * These have a default, generic, SI, copy, move, dtor and copy assign
 * And operator"" and equality and inequality
 * overload +=,-=,+,- with other units of the same type
 * overload *=,/=,*,/ with immediate values and atomics
 * overload ++,-- ?
 * overload >,<,==,!=,<=,>=
 * not overloaded %=,%
 */
class hertz : public measurement<double, SI::prefix, suffix_type_Hz> {
public:
    /** Default constructor */
    hertz();
    /** Parameter constructor */
    explicit hertz(double value);
    /** Copy Constructor */
    hertz(const hertz& other);
    /** Move Constructor */
    hertz(hertz&& other);
    /** Destructor */
    ~hertz() = default;
    /** Const Copy Assignment */
    hertz& operator=(const hertz& other) noexcept;
    /** Copy Assignment */
    hertz& operator=(hertz& other) noexcept;
    /** Move Assignment */
    hertz& operator=(hertz&& other) noexcept;
    /** Equality Operator */
    bool operator==(const hertz& other) const;
    /** Inequality Operator */
    bool operator!=(const hertz& other) const;
    /** Accumulation Operator */
    hertz& operator+=(const hertz& other);
    /** Deccumulation Operator */
    hertz& operator-=(const hertz& other);
    /** Scaling Operator */
    hertz& operator*=(double factor);
    /** Inverse Scaling Operator */
    hertz& operator/=(double factor);
    /** Negating Operator */
    hertz operator-() const;
    /** Less Than Operator */
    bool operator<(const hertz& other) const;
    /** Less Than or Equal Operator */
    bool operator<=(const hertz& other) const;
    /** Greater Than Operator */
    bool operator>(const hertz& other) const;
    /** Greater Than or Equal Operator */
    bool operator>=(const hertz& other) const;
};

namespace operators {
    /** Addition operator */
    hertz operator+(const hertz& A, const hertz& B);
    /** Subtraction operator */
    hertz operator-(const hertz& A, const hertz& B);
    /** Left Scaling Operator */
    hertz operator*(const hertz& A, double factor);
    /** Right Scaling Operator */
    hertz operator*(double factor, const hertz& A);
    /** Inverse Scaling Operator */
    hertz operator/(const hertz& A, double factor);
}

namespace literals {
    /** Literal Double, Quote Operator */
    hertz operator""_Hz(long double value);
    /** Literal Int, Quote Operator */
    hertz operator""_Hz(unsigned long long value);
    /** Literal String, Quote Operator */
    iso::hertz operator""_Hz(const char a[]);
}

}