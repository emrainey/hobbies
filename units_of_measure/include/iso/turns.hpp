/**
 * @file
 * This is an auto-generated header for iso::turns.
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

/** A structure used to hold the postfix for turns */
struct suffix_type_Tau {
    constexpr static const char* suffix = "Tau";
};

/**
 * This is the iso::turns interface.
 * These have a default, generic, SI, copy, move, dtor and copy assign
 * And operator"" and equality and inequality
 * overload +=,-=,+,- with other units of the same type
 * overload *=,/=,*,/ with immediate values and atomics
 * overload ++,-- ?
 * overload >,<,==,!=,<=,>=
 * not overloaded %=,%
 */
class turns : public measurement<double, SI::prefix, suffix_type_Tau> {
public:
    /** Default constructor */
    turns();
    /** Parameter constructor */
    explicit turns(double value);
    /** Copy Constructor */
    turns(const turns& other);
    /** Move Constructor */
    turns(turns&& other);
    /** Destructor */
    ~turns() = default;
    /** Const Copy Assignment */
    turns& operator=(const turns& other) noexcept;
    /** Copy Assignment */
    turns& operator=(turns& other) noexcept;
    /** Move Assignment */
    turns& operator=(turns&& other) noexcept;
    /** Equality Operator */
    bool operator==(const turns& other) const;
    /** Inequality Operator */
    bool operator!=(const turns& other) const;
    /** Accumulation Operator */
    turns& operator+=(const turns& other);
    /** Deccumulation Operator */
    turns& operator-=(const turns& other);
    /** Scaling Operator */
    turns& operator*=(double factor);
    /** Inverse Scaling Operator */
    turns& operator/=(double factor);
    /** Negating Operator */
    turns operator-() const;
    /** Less Than Operator */
    bool operator<(const turns& other) const;
    /** Less Than or Equal Operator */
    bool operator<=(const turns& other) const;
    /** Greater Than Operator */
    bool operator>(const turns& other) const;
    /** Greater Than or Equal Operator */
    bool operator>=(const turns& other) const;
};

namespace operators {
/** Addition operator */
turns operator+(const turns& A, const turns& B);
/** Subtraction operator */
turns operator-(const turns& A, const turns& B);
/** Left Scaling Operator */
turns operator*(const turns& A, double factor);
/** Right Scaling Operator */
turns operator*(double factor, const turns& A);
/** Inverse Scaling Operator */
turns operator/(const turns& A, double factor);
}  // namespace operators

namespace literals {
/** Literal Double, Quote Operator */
turns operator""_Tau(long double value);
/** Literal Int, Quote Operator */
turns operator""_Tau(unsigned long long value);
/** Literal String, Quote Operator */
iso::turns operator""_Tau(const char a[]);
}  // namespace literals

}  // namespace iso