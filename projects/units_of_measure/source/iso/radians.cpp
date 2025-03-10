///
/// @file
/// This is an auto-generated source for iso::radians.
/// A measure of an angle
/// @note See README on re-generation.
/// @copyright Copyright 2025.
///
#include "iso/radians.hpp"

namespace iso {
char const* const suffix_type_rad::suffix;
// default
radians::radians() : measurement{} {
}
// explicit value
radians::radians(precision a) : measurement{a} {
}
// copy constructor
radians::radians(radians const& other) : measurement{other} {
}
// move constructor
radians::radians(radians&& other) : measurement{std::move(other)} {
}

radians& radians::operator=(radians const& other) noexcept {
    _value = other.value;
    return (*this);
}
radians& radians::operator=(radians& other) noexcept {
    _value = other.value;
    return (*this);
}
radians& radians::operator=(radians&& other) noexcept {
    _value = other.value;
    return (*this);
}
bool radians::operator==(radians const& other) const {
    return basal::equivalent(value, other.value);
}
bool radians::operator!=(radians const& other) const {
    return !operator==(other);
}
radians& radians::operator+=(radians const& other) {
    _value += other.value;
    return (*this);
}
radians& radians::operator-=(radians const& other) {
    _value -= other.value;
    return (*this);
}
radians& radians::operator*=(precision factor) {
    _value *= factor;
    return (*this);
}
radians& radians::operator/=(precision factor) {
    _value /= factor;
    return (*this);
}
radians radians::operator-() const {
    return radians{-value};
}
bool radians::operator<(radians const& other) const {
    return (_value < other.value);
}
bool radians::operator<=(radians const& other) const {
    return (_value <= other.value);
}
bool radians::operator>(radians const& other) const {
    return (_value > other.value);
}
bool radians::operator>=(radians const& other) const {
    return (_value >= other.value);
}
namespace operators {
radians operator+(radians const& A, radians const& B) {
    return radians{A.value + B.value};
}
radians operator-(radians const& A, radians const& B) {
    return radians{A.value - B.value};
}
radians operator*(radians const& A, precision factor) {
    return radians{A.value * factor};
}
radians operator*(precision factor, radians const& A) {
    return radians{A.value * factor};
}
radians operator/(radians const& A, precision factor) {
    return radians{A.value / factor};
}
precision operator/(radians const& A, radians const& B) {
    return A.value / B.value;
}
}  // namespace operators
namespace literals {
iso::radians operator""_rad(long double a) {
    return iso::radians(static_cast<precision>(a));
}
iso::radians operator""_rad(unsigned long long a) {
    return iso::radians(static_cast<precision>(a));
}
iso::radians operator""_rad(char const a[]) {
    iso::precision p = static_cast<iso::precision>(atol(a));
    return iso::radians(p);
}
}  // namespace literals
}  // namespace iso

