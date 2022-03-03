/**
 * @file
 * This is an auto-generated header for iso::watts.
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

/** A structure used to hold the postfix for watts */
struct suffix_type_W {
    constexpr static const char* suffix = "W";
};

/**
 * This is the iso::watts interface.
 * These have a default, generic, SI, copy, move, dtor and copy assign
 * And operator"" and equality and inequality
 * overload +=,-=,+,- with other units of the same type
 * overload *=,/=,*,/ with immediate values and atomics
 * overload ++,-- ?
 * overload >,<,==,!=,<=,>=
 * not overloaded %=,%
 */
class watts : public measurement<double, SI::prefix, suffix_type_W> {
public:
    /** Default constructor */
    watts();
    /** Parameter constructor */
    explicit watts(double value);
    /** Copy Constructor */
    watts(const watts& other);
    /** Move Constructor */
    watts(watts&& other);
    /** Destructor */
    ~watts() = default;
    /** Const Copy Assignment */
    watts& operator=(const watts& other) noexcept;
    /** Copy Assignment */
    watts& operator=(watts& other) noexcept;
    /** Move Assignment */
    watts& operator=(watts&& other) noexcept;
    /** Equality Operator */
    bool operator==(const watts& other) const;
    /** Inequality Operator */
    bool operator!=(const watts& other) const;
    /** Accumulation Operator */
    watts& operator+=(const watts& other);
    /** Deccumulation Operator */
    watts& operator-=(const watts& other);
    /** Scaling Operator */
    watts& operator*=(double factor);
    /** Inverse Scaling Operator */
    watts& operator/=(double factor);
    /** Negating Operator */
    watts operator-() const;
    /** Less Than Operator */
    bool operator<(const watts& other) const;
    /** Less Than or Equal Operator */
    bool operator<=(const watts& other) const;
    /** Greater Than Operator */
    bool operator>(const watts& other) const;
    /** Greater Than or Equal Operator */
    bool operator>=(const watts& other) const;
};

namespace operators {
/** Addition operator */
watts operator+(const watts& A, const watts& B);
/** Subtraction operator */
watts operator-(const watts& A, const watts& B);
/** Left Scaling Operator */
watts operator*(const watts& A, double factor);
/** Right Scaling Operator */
watts operator*(double factor, const watts& A);
/** Inverse Scaling Operator */
watts operator/(const watts& A, double factor);
}  // namespace operators

namespace literals {
/** Literal Double, Quote Operator */
watts operator""_W(long double value);
/** Literal Int, Quote Operator */
watts operator""_W(unsigned long long value);
/** Literal String, Quote Operator */
iso::watts operator""_W(const char a[]);
}  // namespace literals

}  // namespace iso