/**
 * @file
 * This is an auto-generated header for iso::newtons.
 * A measure of force
 * @note See README on re-generation.
 * @copyright Copyright 2024.
 */
#pragma once
#include <cstdint>
#include <cstddef>

#include "iso/measurement.hpp"

/** iso comment */
namespace iso {

/** A structure used to hold the postfix for newtons */
struct suffix_type_N {
    constexpr static const char *suffix = "N";
};

/**
 * This is the iso::newtons interface.
 * These have a default, generic, SI, copy, move, dtor and copy assign
 * And operator"" and equality and inequality
 * overload +=,-=,+,- with other units of the same type
 * overload *=,/=,*,/ with immediate values and atomics
 * overload ++,-- ?
 * overload >,<,==,!=,<=,>=
 * not overloaded %=,%
 */
class newtons : public measurement<precision, SI::prefix, suffix_type_N> {
public:
    /** Default constructor */
    newtons();
    /** Parameter constructor */
    explicit newtons(precision value);
    /** Copy Constructor */
    newtons(const newtons& other);
    /** Move Constructor */
    newtons(newtons&& other);
    /** Destructor */
    ~newtons() = default;
    /** Const Copy Assignment */
    newtons& operator=(const newtons& other) noexcept;
    /** Copy Assignment */
    newtons& operator=(newtons& other) noexcept;
    /** Move Assignment */
    newtons& operator=(newtons&& other) noexcept;
    /** Equality Operator */
    bool operator==(const newtons& other) const;
    /** Inequality Operator */
    bool operator!=(const newtons& other) const;
    /** Accumulation Operator */
    newtons& operator+=(const newtons& other);
    /** Deccumulation Operator */
    newtons& operator-=(const newtons& other);
    /** Scaling Operator */
    newtons& operator*=(precision factor);
    /** Inverse Scaling Operator */
    newtons& operator/=(precision factor);
    /** Negating Operator */
    newtons operator-() const;
    /** Less Than Operator */
    bool operator<(const newtons& other) const;
    /** Less Than or Equal Operator */
    bool operator<=(const newtons& other) const;
    /** Greater Than Operator */
    bool operator>(const newtons& other) const;
    /** Greater Than or Equal Operator */
    bool operator>=(const newtons& other) const;
};

namespace operators {
    /** Addition operator */
    newtons operator+(const newtons& A, const newtons& B);
    /** Subtraction operator */
    newtons operator-(const newtons& A, const newtons& B);
    /** Left Scaling Operator */
    newtons operator*(const newtons& A, precision factor);
    /** Right Scaling Operator */
    newtons operator*(precision factor, const newtons& A);
    /** Inverse Scaling Operator */
    newtons operator/(const newtons& A, precision factor);
}

namespace literals {
    /** Literal Double, Quote Operator */
    newtons operator""_N(long double value);
    /** Literal Int, Quote Operator */
    newtons operator""_N(unsigned long long value);
    /** Literal String, Quote Operator */
    iso::newtons operator""_N(const char a[]);
}

}