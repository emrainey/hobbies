/**
 * @file
 * This is an auto-generated header for iso::joules.
 * 
 * @note See README on re-generation.
 * @copyright Copyright 2022.
 */
#pragma once
#include <cstdint>
#include <cstddef>

#include "iso/measurement.hpp"

/** iso comment */
namespace iso {

/** A structure used to hold the postfix for joules */
struct suffix_type_J {
    constexpr static const char *suffix = "J";
};

/**
 * This is the iso::joules interface.
 * These have a default, generic, SI, copy, move, dtor and copy assign
 * And operator"" and equality and inequality
 * overload +=,-=,+,- with other units of the same type
 * overload *=,/=,*,/ with immediate values and atomics
 * overload ++,-- ?
 * overload >,<,==,!=,<=,>=
 * not overloaded %=,%
 */
class joules : public measurement<double, SI::prefix, suffix_type_J> {
public:
    /** Default constructor */
    joules();
    /** Parameter constructor */
    explicit joules(double value);
    /** Copy Constructor */
    joules(const joules& other);
    /** Move Constructor */
    joules(joules&& other);
    /** Destructor */
    ~joules() = default;
    /** Const Copy Assignment */
    joules& operator=(const joules &other) noexcept;
    /** Copy Assignment */
    joules& operator=(joules& other) noexcept;
    /** Move Assignment */
    joules& operator=(joules&& other) noexcept;
    /** Equality Operator */
    bool operator==(const joules& other) const;
    /** Inequality Operator */
    bool operator!=(const joules& other) const;
    /** Accumulation Operator */
    joules& operator+=(const joules& other);
    /** Deccumulation Operator */
    joules& operator-=(const joules& other);
    /** Scaling Operator */
    joules& operator*=(double factor);
    /** Inverse Scaling Operator */
    joules& operator/=(double factor);
    /** Negating Operator */
    joules operator-() const;
    /** Less Than Operator */
    bool operator<(const joules& other) const;
    /** Less Than or Equal Operator */
    bool operator<=(const joules& other) const;
    /** Greater Than Operator */
    bool operator>(const joules& other) const;
    /** Greater Than or Equal Operator */
    bool operator>=(const joules& other) const;
};

namespace operators {
    /** Addition operator */
    joules operator+(const joules& A, const joules& B);
    /** Subtraction operator */
    joules operator-(const joules& A, const joules& B);
    /** Left Scaling Operator */
    joules operator*(const joules& A, double factor);
    /** Right Scaling Operator */
    joules operator*(double factor, const joules& A);
    /** Inverse Scaling Operator */
    joules operator/(const joules& A, double factor);
}

namespace literals {
    /** Literal Double, Quote Operator */
    joules operator""_J(long double value);
    /** Literal Int, Quote Operator */
    joules operator""_J(unsigned long long value);
    /** Literal String, Quote Operator */
    iso::joules operator""_J(const char a[]);
}

}