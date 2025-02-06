///
/// @file
/// This is an auto-generated source for iso::volts.
/// A measure of the electrical potential
/// @note See README on re-generation.
/// @copyright Copyright 2025.
///
#include "iso/volts.hpp"

namespace iso {
char const* const suffix_type_V::suffix;
// default
volts::volts() : measurement{} {
}
// explicit value
volts::volts(precision a) : measurement{a} {
}
// copy constructor
volts::volts(volts const& other) : measurement{other} {
}
// move constructor
volts::volts(volts&& other) : measurement{std::move(other)} {
}

volts& volts::operator=(volts const& other) noexcept {
    _value = other.value;
    return (*this);
}
volts& volts::operator=(volts& other) noexcept {
    _value = other.value;
    return (*this);
}
volts& volts::operator=(volts&& other) noexcept {
    _value = other.value;
    return (*this);
}
bool volts::operator==(volts const& other) const {
    return basal::equivalent(value, other.value);
}
bool volts::operator!=(volts const& other) const {
    return !operator==(other);
}
volts& volts::operator+=(volts const& other) {
    _value += other.value;
    return (*this);
}
volts& volts::operator-=(volts const& other) {
    _value -= other.value;
    return (*this);
}
volts& volts::operator*=(precision factor) {
    _value *= factor;
    return (*this);
}
volts& volts::operator/=(precision factor) {
    _value /= factor;
    return (*this);
}
volts volts::operator-() const {
    return volts{-value};
}
bool volts::operator<(volts const& other) const {
    return (_value < other.value);
}
bool volts::operator<=(volts const& other) const {
    return (_value <= other.value);
}
bool volts::operator>(volts const& other) const {
    return (_value > other.value);
}
bool volts::operator>=(volts const& other) const {
    return (_value >= other.value);
}
namespace operators {
volts operator+(volts const& A, volts const& B) {
    return volts{A.value + B.value};
}
volts operator-(volts const& A, volts const& B) {
    return volts{A.value - B.value};
}
volts operator*(volts const& A, precision factor) {
    return volts{A.value * factor};
}
volts operator*(precision factor, volts const& A) {
    return volts{A.value * factor};
}
volts operator/(volts const& A, precision factor) {
    return volts{A.value / factor};
}
precision operator/(volts const& A, volts const& B) {
    return A.value / B.value;
}
}  // namespace operators
namespace literals {
iso::volts operator""_V(long double a) {
    return iso::volts(static_cast<precision>(a));
}
iso::volts operator""_V(unsigned long long a) {
    return iso::volts(a);
}
iso::volts operator""_V(char const a[]) {
    return iso::volts(atol(a));
}
}  // namespace literals
}  // namespace iso

