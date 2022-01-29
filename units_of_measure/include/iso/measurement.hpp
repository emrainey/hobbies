#pragma once

#include "si/prefix.hpp"
#include "iec/prefix.hpp"
#include <ostream>
#include <cmath>

/** The ISO namespace */
namespace iso {

// A simple float/double equals
inline bool equivalent(const double a, const double b) {
    return not(a > b) and not(a < b);
}

/**
 * Converts the value to another scale.
 */
template <typename UNIT_TYPE, typename SCALE_TYPE>
UNIT_TYPE rescale(UNIT_TYPE value, SCALE_TYPE old_scale, SCALE_TYPE new_scale);

template <typename UNIT_TYPE, typename SCALE_TYPE, typename SUFFIX_TYPE>
class measurement {
protected:
    /** The protected internal value */
    UNIT_TYPE _value;
public:
    /** A read-only reference of the value at the default scale */
    const UNIT_TYPE& value;
protected:
    /** The constant string which has our unit */
    const char * const _suffix;
public:
    /** The declared type of the value */
    using type = UNIT_TYPE;

    /** Default Constructor */
    measurement() : _value(0), value(_value), _suffix(SUFFIX_TYPE::suffix) {}
    /** Valued Constructor */
    measurement(UNIT_TYPE v) : _value(v), value(_value), _suffix(SUFFIX_TYPE::suffix) {}
    /** Const Copy Constructor */
    measurement(const measurement& other) : _value(other.value), value(_value), _suffix(other._suffix) {}
    /** Copy Constructor */
    measurement(measurement& other) : _value(other.value), value(_value), _suffix(other._suffix) {}
    /** Move Constructor */
    measurement(measurement &&other) : _value(other.value), value(_value), _suffix(std::move(other._suffix)) {}
    /** Destructor */
    virtual ~measurement() {}

    // Don't allow one measurement to be transferred to another by default,
    // Child classes must overload any operators

    /** Returns the units as a string */
    const char * get_suffix() const { return _suffix; }

    /** Returns the value at a new scale */
    UNIT_TYPE at_scale(SCALE_TYPE new_scale) const {
        return rescale(_value, SCALE_TYPE::none, new_scale);
    }

    /** Returns the value at the default scale */
    UNIT_TYPE operator()() const {
        return _value;
    }

    //friend std::ostream& operator<<(std::ostream& output, const iso::measurement& obj);
};
}

/** This allow the measurement object to be able to be printed via streams */
template <typename UNIT_TYPE, typename SCALE_TYPE, typename SUFFIX_TYPE>
std::ostream& operator<<(std::ostream& output, const iso::measurement<UNIT_TYPE, SCALE_TYPE, SUFFIX_TYPE>& obj) {
    return output << obj.value << "_" << obj.get_suffix();
}

#define DEFINE_MEASUREMENT_CLASS(CLASSNAME, VALUE_TYPE, SCALE_TYPE, __SUFFIX__) \
namespace iso { \
struct suffix_type_##__SUFFIX__ { \
    constexpr static const char *suffix = #__SUFFIX__ ; \
}; \
class CLASSNAME : public measurement<VALUE_TYPE, SCALE_TYPE, suffix_type_##__SUFFIX__> { \
public: \
    CLASSNAME(); \
    explicit CLASSNAME(VALUE_TYPE a); \
    CLASSNAME(const CLASSNAME &other); \
    CLASSNAME(CLASSNAME &&other); \
    CLASSNAME& operator=(const CLASSNAME &other); \
    CLASSNAME& operator=(CLASSNAME& other); \
    CLASSNAME& operator=(CLASSNAME&& other); \
    bool operator==(const CLASSNAME& other) const; \
    bool operator!=(const CLASSNAME& other) const; \
    CLASSNAME& operator+=(const CLASSNAME& other); \
    CLASSNAME& operator-=(const CLASSNAME& other); \
    CLASSNAME& operator*=(VALUE_TYPE factor); \
    CLASSNAME& operator/=(VALUE_TYPE factor); \
    CLASSNAME operator-() const; \
    bool operator<(const CLASSNAME& other) const; \
    bool operator<=(const CLASSNAME& other) const; \
    bool operator>(const CLASSNAME& other) const; \
    bool operator>=(const CLASSNAME& other) const; \
    virtual ~CLASSNAME(); \
}; \
CLASSNAME operator+(const CLASSNAME& A, const CLASSNAME& B); \
CLASSNAME operator-(const CLASSNAME& A, const CLASSNAME& B); \
CLASSNAME operator*(const CLASSNAME& A, VALUE_TYPE factor); \
CLASSNAME operator*(VALUE_TYPE factor, const CLASSNAME& A); \
CLASSNAME operator/(const CLASSNAME& A, VALUE_TYPE factor); \
} \
iso::CLASSNAME operator""_##__SUFFIX__(const char desc[], std::size_t len); \
iso::CLASSNAME operator""_##__SUFFIX__(unsigned long long a); \
iso::CLASSNAME operator""_##__SUFFIX__(long double a); \

// These have a default, generic, SI, copy, move, dtor and copy assign
// And operator"" and equality and inequality
// overload +=,-=,+,- with other units of the same type
// overload *=,/=,*,/ with immediate values and atomics
// overload ++,-- ?
// overload >,<,==,!=,<=,>=
// not overloaded %=,%
#define DECLARE_MEASUREMENT_CLASS(CLASSNAME, VALUE_TYPE, SCALE_TYPE, __SUFFIX__) \
namespace iso { \
const char *const suffix_type_##__SUFFIX__::suffix; \
CLASSNAME::CLASSNAME() : measurement() {} \
CLASSNAME::CLASSNAME(VALUE_TYPE a) : measurement(a) {} \
CLASSNAME::CLASSNAME(const CLASSNAME &other) : measurement(other) {} \
CLASSNAME::CLASSNAME(CLASSNAME &&other) : measurement(std::move(other)) {} \
CLASSNAME& CLASSNAME::operator=(const CLASSNAME &other) { \
    _value = other.value; \
    return (*this); \
} \
CLASSNAME& CLASSNAME::operator=(CLASSNAME &other) { \
    _value = other.value; \
    return (*this); \
} \
CLASSNAME& CLASSNAME::operator=(CLASSNAME &&other) { \
    _value = other.value; \
    return (*this); \
} \
bool CLASSNAME::operator==(const CLASSNAME &other) const { \
    return iso::equivalent(value, other.value); \
} \
bool CLASSNAME::operator!=(const CLASSNAME &other) const { \
    return !operator==(other); \
} \
CLASSNAME& CLASSNAME::operator+=(const CLASSNAME& other) { \
    _value += other.value; \
    return (*this); \
} \
CLASSNAME& CLASSNAME::operator-=(const CLASSNAME& other) { \
    _value -= other.value; \
    return (*this); \
} \
CLASSNAME& CLASSNAME::operator*=(VALUE_TYPE factor) { \
    _value *= factor; \
    return (*this); \
} \
CLASSNAME& CLASSNAME::operator/=(VALUE_TYPE factor) { \
    _value /= factor; \
    return (*this); \
} \
CLASSNAME CLASSNAME::operator-() const { \
    return CLASSNAME(-value); \
} \
bool CLASSNAME::operator<(const CLASSNAME& other) const { \
    return (_value < other.value); \
} \
bool CLASSNAME::operator<=(const CLASSNAME& other) const { \
    return (_value <= other.value); \
} \
bool CLASSNAME::operator>(const CLASSNAME& other) const { \
    return (_value > other.value); \
} \
bool CLASSNAME::operator>=(const CLASSNAME& other) const { \
    return (_value >= other.value); \
} \
CLASSNAME::~CLASSNAME() {} \
CLASSNAME operator+(const CLASSNAME& A, const CLASSNAME& B) { return CLASSNAME(A.value+B.value); } \
CLASSNAME operator-(const CLASSNAME& A, const CLASSNAME& B) { return CLASSNAME(A.value-B.value); } \
CLASSNAME operator*(const CLASSNAME& A, VALUE_TYPE factor)         { return CLASSNAME(A.value*factor); } \
CLASSNAME operator*(VALUE_TYPE factor, const CLASSNAME& A)         { return CLASSNAME(A.value*factor); } \
CLASSNAME operator/(const CLASSNAME& A, VALUE_TYPE factor)         { return CLASSNAME(A.value/factor); } \
} /* end of namespace */ \
iso::CLASSNAME operator""_##__SUFFIX__(long double a)        { return iso::CLASSNAME(a); } \
iso::CLASSNAME operator""_##__SUFFIX__(unsigned long long a) { return iso::CLASSNAME(a); } \
iso::CLASSNAME operator""_##__SUFFIX__(const char a[])       { return iso::CLASSNAME(atol(a)); }
