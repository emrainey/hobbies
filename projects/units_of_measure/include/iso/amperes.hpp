/**
 * @file
 * This is an auto-generated header for iso::amperes.
 * A measure of current in electricity
 * @note See README on re-generation.
 * @copyright Copyright 2022.
 */
#pragma once
#include <cstdint>
#include <cstddef>

#include "iso/measurement.hpp"

/** iso comment */
namespace iso {

/** A structure used to hold the postfix for amperes */
struct suffix_type_A {
    constexpr static const char *suffix = "A";
};

/**
 * This is the iso::amperes interface.
 * These have a default, generic, SI, copy, move, dtor and copy assign
 * And operator"" and equality and inequality
 * overload +=,-=,+,- with other units of the same type
 * overload *=,/=,*,/ with immediate values and atomics
 * overload ++,-- ?
 * overload >,<,==,!=,<=,>=
 * not overloaded %=,%
 */
class amperes : public measurement<double, SI::prefix, suffix_type_A> {
public:
    /** Default constructor */
    amperes();
    /** Parameter constructor */
    explicit amperes(double value);
    /** Copy Constructor */
    amperes(const amperes& other);
    /** Move Constructor */
    amperes(amperes&& other);
    /** Destructor */
    ~amperes() = default;
    /** Const Copy Assignment */
    amperes& operator=(const amperes& other) noexcept;
    /** Copy Assignment */
    amperes& operator=(amperes& other) noexcept;
    /** Move Assignment */
    amperes& operator=(amperes&& other) noexcept;
    /** Equality Operator */
    bool operator==(const amperes& other) const;
    /** Inequality Operator */
    bool operator!=(const amperes& other) const;
    /** Accumulation Operator */
    amperes& operator+=(const amperes& other);
    /** Deccumulation Operator */
    amperes& operator-=(const amperes& other);
    /** Scaling Operator */
    amperes& operator*=(double factor);
    /** Inverse Scaling Operator */
    amperes& operator/=(double factor);
    /** Negating Operator */
    amperes operator-() const;
    /** Less Than Operator */
    bool operator<(const amperes& other) const;
    /** Less Than or Equal Operator */
    bool operator<=(const amperes& other) const;
    /** Greater Than Operator */
    bool operator>(const amperes& other) const;
    /** Greater Than or Equal Operator */
    bool operator>=(const amperes& other) const;
};

namespace operators {
    /** Addition operator */
    amperes operator+(const amperes& A, const amperes& B);
    /** Subtraction operator */
    amperes operator-(const amperes& A, const amperes& B);
    /** Left Scaling Operator */
    amperes operator*(const amperes& A, double factor);
    /** Right Scaling Operator */
    amperes operator*(double factor, const amperes& A);
    /** Inverse Scaling Operator */
    amperes operator/(const amperes& A, double factor);
}

namespace literals {
    /** Literal Double, Quote Operator */
    amperes operator""_A(long double value);
    /** Literal Int, Quote Operator */
    amperes operator""_A(unsigned long long value);
    /** Literal String, Quote Operator */
    iso::amperes operator""_A(const char a[]);
}

}