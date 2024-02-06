///
/// @file
/// This is an auto-generated source for iso::hertz.
/// A measure of frequency
/// @note See README on re-generation.
/// @copyright Copyright 2024.
///
#include "iso/hertz.hpp"

namespace iso {
const char* const suffix_type_Hz::suffix;
// default
hertz::hertz() : measurement{} {
}
// explicit value
hertz::hertz(precision a) : measurement{a} {
}
// copy constructor
hertz::hertz(const hertz& other) : measurement{other} {
}
// move constructor
hertz::hertz(hertz&& other) : measurement{std::move(other)} {
}

hertz& hertz::operator=(const hertz &other) noexcept {
    _value = other.value;
    return (*this);
}
hertz& hertz::operator=(hertz& other) noexcept {
    _value = other.value;
    return (*this);
}
hertz& hertz::operator=(hertz&& other) noexcept {
    _value = other.value;
    return (*this);
}
bool hertz::operator==(const hertz& other) const {
    return basal::equivalent(value, other.value);
}
bool hertz::operator!=(const hertz& other) const {
    return !operator==(other);
}
hertz& hertz::operator+=(const hertz& other) {
    _value += other.value;
    return (*this);
}
hertz& hertz::operator-=(const hertz& other) {
    _value -= other.value;
    return (*this);
}
hertz& hertz::operator*=(precision factor) {
    _value *= factor;
    return (*this);
}
hertz& hertz::operator/=(precision factor) {
    _value /= factor;
    return (*this);
}
hertz hertz::operator-() const {
    return hertz{-value};
}
bool hertz::operator<(const hertz& other) const {
    return (_value < other.value);
}
bool hertz::operator<=(const hertz& other) const {
    return (_value <= other.value);
}
bool hertz::operator>(const hertz& other) const {
    return (_value > other.value);
}
bool hertz::operator>=(const hertz& other) const {
    return (_value >= other.value);
}
namespace operators {
hertz operator+(const hertz& A, const hertz& B) {
    return hertz{A.value + B.value};
}
hertz operator-(const hertz& A, const hertz& B) {
    return hertz{A.value - B.value};
}
hertz operator*(const hertz& A, precision factor) {
    return hertz{A.value * factor};
}
hertz operator*(precision factor, const hertz& A) {
    return hertz{A.value * factor};
}
hertz operator/(const hertz& A, precision factor) {
    return hertz{A.value / factor};
}
}  // namespace operators
namespace literals {
iso::hertz operator""_Hz(long double a) {
    return iso::hertz(static_cast<precision>(a));
}
iso::hertz operator""_Hz(unsigned long long a) {
    return iso::hertz(a);
}
iso::hertz operator""_Hz(const char a[]) {
    return iso::hertz(atol(a));
}
}  // namespace literals
}  // namespace iso

