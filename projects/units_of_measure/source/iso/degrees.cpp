/**
 * @file
 * This is an auto-generated source for iso::degrees.
 * A measure of an angle
 * @note See README on re-generation.
 * @copyright Copyright 2022.
 */
#include "iso/degrees.hpp"

namespace iso {
const char* const suffix_type_d::suffix;
// default
degrees::degrees() : measurement{} {
}
// explicit value
degrees::degrees(double a) : measurement{a} {
}
// copy constructor
degrees::degrees(const degrees& other) : measurement{other} {
}
// move constructor
degrees::degrees(degrees&& other) : measurement{std::move(other)} {
}

degrees& degrees::operator=(const degrees &other) noexcept {
    _value = other.value;
    return (*this);
}
degrees& degrees::operator=(degrees& other) noexcept {
    _value = other.value;
    return (*this);
}
degrees& degrees::operator=(degrees&& other) noexcept {
    _value = other.value;
    return (*this);
}
bool degrees::operator==(const degrees& other) const {
    return iso::equivalent(value, other.value);
}
bool degrees::operator!=(const degrees& other) const {
    return !operator==(other);
}
degrees& degrees::operator+=(const degrees& other) {
    _value += other.value;
    return (*this);
}
degrees& degrees::operator-=(const degrees& other) {
    _value -= other.value;
    return (*this);
}
degrees& degrees::operator*=(double factor) {
    _value *= factor;
    return (*this);
}
degrees& degrees::operator/=(double factor) {
    _value /= factor;
    return (*this);
}
degrees degrees::operator-() const {
    return degrees{-value};
}
bool degrees::operator<(const degrees& other) const {
    return (_value < other.value);
}
bool degrees::operator<=(const degrees& other) const {
    return (_value <= other.value);
}
bool degrees::operator>(const degrees& other) const {
    return (_value > other.value);
}
bool degrees::operator>=(const degrees& other) const {
    return (_value >= other.value);
}
namespace operators {
degrees operator+(const degrees& A, const degrees& B) {
    return degrees{A.value + B.value};
}
degrees operator-(const degrees& A, const degrees& B) {
    return degrees{A.value - B.value};
}
degrees operator*(const degrees& A, double factor) {
    return degrees{A.value * factor};
}
degrees operator*(double factor, const degrees& A) {
    return degrees{A.value * factor};
}
degrees operator/(const degrees& A, double factor) {
    return degrees{A.value / factor};
}
}  // namespace operators
namespace literals {
iso::degrees operator""_d(long double a) {
    return iso::degrees(a);
}
iso::degrees operator""_d(unsigned long long a) {
    return iso::degrees(a);
}
iso::degrees operator""_d(const char a[]) {
    return iso::degrees(atol(a));
}
}  // namespace literals
}  // namespace iso

