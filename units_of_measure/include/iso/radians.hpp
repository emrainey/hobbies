/**
 * @file
 * This is an auto-generated header for iso::radians.
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

/** A structure used to hold the postfix for radians */
struct suffix_type_rad {
    constexpr static const char *suffix = "rad";
};

/**
 * This is the iso::radians interface.
 * These have a default, generic, SI, copy, move, dtor and copy assign
 * And operator"" and equality and inequality
 * overload +=,-=,+,- with other units of the same type
 * overload *=,/=,*,/ with immediate values and atomics
 * overload ++,-- ?
 * overload >,<,==,!=,<=,>=
 * not overloaded %=,%
 */
class radians : public measurement<double, SI::prefix, suffix_type_rad> {
public:
    /** Default constructor */
    radians();
    /** Parameter constructor */
    explicit radians(double value);
    /** Copy Constructor */
    radians(const radians& other);
    /** Move Constructor */
    radians(radians&& other);
    /** Destructor */
    ~radians() = default;
    /** Const Copy Assignment */
    radians& operator=(const radians &other) noexcept;
    /** Copy Assignment */
    radians& operator=(radians& other) noexcept;
    /** Move Assignment */
    radians& operator=(radians&& other) noexcept;
    /** Equality Operator */
    bool operator==(const radians& other) const;
    /** Inequality Operator */
    bool operator!=(const radians& other) const;
    /** Accumulation Operator */
    radians& operator+=(const radians& other);
    /** Deccumulation Operator */
    radians& operator-=(const radians& other);
    /** Scaling Operator */
    radians& operator*=(double factor);
    /** Inverse Scaling Operator */
    radians& operator/=(double factor);
    /** Negating Operator */
    radians operator-() const;
    /** Less Than Operator */
    bool operator<(const radians& other) const;
    /** Less Than or Equal Operator */
    bool operator<=(const radians& other) const;
    /** Greater Than Operator */
    bool operator>(const radians& other) const;
    /** Greater Than or Equal Operator */
    bool operator>=(const radians& other) const;
};

namespace operators {
    /** Addition operator */
    radians operator+(const radians& A, const radians& B);
    /** Subtraction operator */
    radians operator-(const radians& A, const radians& B);
    /** Left Scaling Operator */
    radians operator*(const radians& A, double factor);
    /** Right Scaling Operator */
    radians operator*(double factor, const radians& A);
    /** Inverse Scaling Operator */
    radians operator/(const radians& A, double factor);
}

namespace literals {
    /** Literal Double, Quote Operator */
    radians operator""_rad(long double value);
    /** Literal Int, Quote Operator */
    radians operator""_rad(unsigned long long value);
    /** Literal String, Quote Operator */
    iso::radians operator""_rad(const char a[]);
}

}