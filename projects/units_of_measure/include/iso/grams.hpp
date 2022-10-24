/**
 * @file
 * This is an auto-generated header for iso::grams.
 * A measurement of mass
 * @note See README on re-generation.
 * @copyright Copyright 2022.
 */
#pragma once
#include <cstdint>
#include <cstddef>

#include "iso/measurement.hpp"

/** iso comment */
namespace iso {

/** A structure used to hold the postfix for grams */
struct suffix_type_g {
    constexpr static const char *suffix = "g";
};

/**
 * This is the iso::grams interface.
 * These have a default, generic, SI, copy, move, dtor and copy assign
 * And operator"" and equality and inequality
 * overload +=,-=,+,- with other units of the same type
 * overload *=,/=,*,/ with immediate values and atomics
 * overload ++,-- ?
 * overload >,<,==,!=,<=,>=
 * not overloaded %=,%
 */
class grams : public measurement<double, SI::prefix, suffix_type_g> {
public:
    /** Default constructor */
    grams();
    /** Parameter constructor */
    explicit grams(double value);
    /** Copy Constructor */
    grams(const grams& other);
    /** Move Constructor */
    grams(grams&& other);
    /** Destructor */
    ~grams() = default;
    /** Const Copy Assignment */
    grams& operator=(const grams& other) noexcept;
    /** Copy Assignment */
    grams& operator=(grams& other) noexcept;
    /** Move Assignment */
    grams& operator=(grams&& other) noexcept;
    /** Equality Operator */
    bool operator==(const grams& other) const;
    /** Inequality Operator */
    bool operator!=(const grams& other) const;
    /** Accumulation Operator */
    grams& operator+=(const grams& other);
    /** Deccumulation Operator */
    grams& operator-=(const grams& other);
    /** Scaling Operator */
    grams& operator*=(double factor);
    /** Inverse Scaling Operator */
    grams& operator/=(double factor);
    /** Negating Operator */
    grams operator-() const;
    /** Less Than Operator */
    bool operator<(const grams& other) const;
    /** Less Than or Equal Operator */
    bool operator<=(const grams& other) const;
    /** Greater Than Operator */
    bool operator>(const grams& other) const;
    /** Greater Than or Equal Operator */
    bool operator>=(const grams& other) const;
};

namespace operators {
    /** Addition operator */
    grams operator+(const grams& A, const grams& B);
    /** Subtraction operator */
    grams operator-(const grams& A, const grams& B);
    /** Left Scaling Operator */
    grams operator*(const grams& A, double factor);
    /** Right Scaling Operator */
    grams operator*(double factor, const grams& A);
    /** Inverse Scaling Operator */
    grams operator/(const grams& A, double factor);
}

namespace literals {
    /** Literal Double, Quote Operator */
    grams operator""_g(long double value);
    /** Literal Int, Quote Operator */
    grams operator""_g(unsigned long long value);
    /** Literal String, Quote Operator */
    iso::grams operator""_g(const char a[]);
}

}