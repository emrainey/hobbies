/**
 * @file
 * This is an auto-generated source for iso::radians.
 * 
 * @note See README on re-generation.
 * @copyright Copyright 2022.
 */
#include "iso/radians.hpp"

namespace iso {
const char *const suffix_type_rad::suffix;
// default
radians::radians() : measurement() {}
// explicit value
radians::radians(double a) : measurement(a) {}
// copy constructor
radians::radians(const radians& other) : measurement(other) {}
// move constructor
radians::radians(radians&& other) : measurement(std::move(other)) {}

radians& radians::operator=(const radians &other) noexcept {
    _value = other.value;
    return (*this);
}
radians& radians::operator=(radians &other) noexcept {
    _value = other.value;
    return (*this);
}
radians& radians::operator=(radians &&other) noexcept {
    _value = other.value;
    return (*this);
}
bool radians::operator==(const radians &other) const {
    return iso::equivalent(value, other.value);
}
bool radians::operator!=(const radians &other) const {
    return !operator==(other);
}
radians& radians::operator+=(const radians& other) {
    _value += other.value;
    return (*this);
}
radians& radians::operator-=(const radians& other) {
    _value -= other.value;
    return (*this);
}
radians& radians::operator*=(double factor) {
    _value *= factor;
    return (*this);
}
radians& radians::operator/=(double factor) {
    _value /= factor;
    return (*this);
}
radians radians::operator-() const {
    return radians(-value);
}
bool radians::operator<(const radians& other) const {
    return (_value < other.value);
}
bool radians::operator<=(const radians& other) const {
    return (_value <= other.value);
}
bool radians::operator>(const radians& other) const {
    return (_value > other.value);
}
bool radians::operator>=(const radians& other) const {
    return (_value >= other.value);
}
namespace operators {
    radians operator+(const radians& A, const radians& B) {
        return radians(A.value+B.value);
    }
    radians operator-(const radians& A, const radians& B) {
        return radians(A.value-B.value);
    }
    radians operator*(const radians& A, double factor) {
        return radians(A.value*factor);
    }
    radians operator*(double factor, const radians& A) {
        return radians(A.value*factor);
    }
    radians operator/(const radians& A, double factor) {
        return radians(A.value/factor);
    }
}
namespace literals {
    iso::radians operator""_rad(long double a) {
        return iso::radians(a);
    }
    iso::radians operator""_rad(unsigned long long a) {
        return iso::radians(a);
    }
    iso::radians operator""_rad(const char a[]) {
        return iso::radians(atol(a));
    }
} // namespace literals
} // namespace iso

