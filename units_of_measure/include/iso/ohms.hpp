/**
 * @file
 * This is an auto-generated header for iso::ohms.
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

/** A structure used to hold the postfix for ohms */
struct suffix_type_Ohm {
    constexpr static const char* suffix = "Ohm";
};

/**
 * This is the iso::ohms interface.
 * These have a default, generic, SI, copy, move, dtor and copy assign
 * And operator"" and equality and inequality
 * overload +=,-=,+,- with other units of the same type
 * overload *=,/=,*,/ with immediate values and atomics
 * overload ++,-- ?
 * overload >,<,==,!=,<=,>=
 * not overloaded %=,%
 */
class ohms : public measurement<double, SI::prefix, suffix_type_Ohm> {
public:
    /** Default constructor */
    ohms();
    /** Parameter constructor */
    explicit ohms(double value);
    /** Copy Constructor */
    ohms(const ohms& other);
    /** Move Constructor */
    ohms(ohms&& other);
    /** Destructor */
    ~ohms() = default;
    /** Const Copy Assignment */
    ohms& operator=(const ohms& other) noexcept;
    /** Copy Assignment */
    ohms& operator=(ohms& other) noexcept;
    /** Move Assignment */
    ohms& operator=(ohms&& other) noexcept;
    /** Equality Operator */
    bool operator==(const ohms& other) const;
    /** Inequality Operator */
    bool operator!=(const ohms& other) const;
    /** Accumulation Operator */
    ohms& operator+=(const ohms& other);
    /** Deccumulation Operator */
    ohms& operator-=(const ohms& other);
    /** Scaling Operator */
    ohms& operator*=(double factor);
    /** Inverse Scaling Operator */
    ohms& operator/=(double factor);
    /** Negating Operator */
    ohms operator-() const;
    /** Less Than Operator */
    bool operator<(const ohms& other) const;
    /** Less Than or Equal Operator */
    bool operator<=(const ohms& other) const;
    /** Greater Than Operator */
    bool operator>(const ohms& other) const;
    /** Greater Than or Equal Operator */
    bool operator>=(const ohms& other) const;
};

namespace operators {
/** Addition operator */
ohms operator+(const ohms& A, const ohms& B);
/** Subtraction operator */
ohms operator-(const ohms& A, const ohms& B);
/** Left Scaling Operator */
ohms operator*(const ohms& A, double factor);
/** Right Scaling Operator */
ohms operator*(double factor, const ohms& A);
/** Inverse Scaling Operator */
ohms operator/(const ohms& A, double factor);
}  // namespace operators

namespace literals {
/** Literal Double, Quote Operator */
ohms operator""_Ohm(long double value);
/** Literal Int, Quote Operator */
ohms operator""_Ohm(unsigned long long value);
/** Literal String, Quote Operator */
iso::ohms operator""_Ohm(const char a[]);
}  // namespace literals

}  // namespace iso