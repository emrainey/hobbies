/**
 * @file
 * This is an auto-generated source for iso::volts.
 *
 * @note See README on re-generation.
 * @copyright Copyright 2022.
 */
#include "iso/volts.hpp"

namespace iso {
const char* const suffix_type_V::suffix;
// default
volts::volts() : measurement() {
}
// explicit value
volts::volts(double a) : measurement(a) {
}
// copy constructor
volts::volts(const volts& other) : measurement(other) {
}
// move constructor
volts::volts(volts&& other) : measurement(std::move(other)) {
}

volts& volts::operator=(const volts& other) noexcept {
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
bool volts::operator==(const volts& other) const {
    return iso::equivalent(value, other.value);
}
bool volts::operator!=(const volts& other) const {
    return !operator==(other);
}
volts& volts::operator+=(const volts& other) {
    _value += other.value;
    return (*this);
}
volts& volts::operator-=(const volts& other) {
    _value -= other.value;
    return (*this);
}
volts& volts::operator*=(double factor) {
    _value *= factor;
    return (*this);
}
volts& volts::operator/=(double factor) {
    _value /= factor;
    return (*this);
}
volts volts::operator-() const {
    return volts(-value);
}
bool volts::operator<(const volts& other) const {
    return (_value < other.value);
}
bool volts::operator<=(const volts& other) const {
    return (_value <= other.value);
}
bool volts::operator>(const volts& other) const {
    return (_value > other.value);
}
bool volts::operator>=(const volts& other) const {
    return (_value >= other.value);
}
namespace operators {
volts operator+(const volts& A, const volts& B) {
    return volts(A.value + B.value);
}
volts operator-(const volts& A, const volts& B) {
    return volts(A.value - B.value);
}
volts operator*(const volts& A, double factor) {
    return volts(A.value * factor);
}
volts operator*(double factor, const volts& A) {
    return volts(A.value * factor);
}
volts operator/(const volts& A, double factor) {
    return volts(A.value / factor);
}
}  // namespace operators
namespace literals {
iso::volts operator""_V(long double a) {
    return iso::volts(a);
}
iso::volts operator""_V(unsigned long long a) {
    return iso::volts(a);
}
iso::volts operator""_V(const char a[]) {
    return iso::volts(atol(a));
}
}  // namespace literals
}  // namespace iso
