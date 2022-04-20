/**
 * @file
 * This is an auto-generated source for iso::meters.
 *
 * @note See README on re-generation.
 * @copyright Copyright 2022.
 */
#include "iso/meters.hpp"

namespace iso {
const char* const suffix_type_m::suffix;
// default
meters::meters() : measurement() {
}
// explicit value
meters::meters(double a) : measurement(a) {
}
// copy constructor
meters::meters(const meters& other) : measurement(other) {
}
// move constructor
meters::meters(meters&& other) : measurement(std::move(other)) {
}

meters& meters::operator=(const meters& other) noexcept {
    _value = other.value;
    return (*this);
}
meters& meters::operator=(meters& other) noexcept {
    _value = other.value;
    return (*this);
}
meters& meters::operator=(meters&& other) noexcept {
    _value = other.value;
    return (*this);
}
bool meters::operator==(const meters& other) const {
    return iso::equivalent(value, other.value);
}
bool meters::operator!=(const meters& other) const {
    return !operator==(other);
}
meters& meters::operator+=(const meters& other) {
    _value += other.value;
    return (*this);
}
meters& meters::operator-=(const meters& other) {
    _value -= other.value;
    return (*this);
}
meters& meters::operator*=(double factor) {
    _value *= factor;
    return (*this);
}
meters& meters::operator/=(double factor) {
    _value /= factor;
    return (*this);
}
meters meters::operator-() const {
    return meters{-value};
}
bool meters::operator<(const meters& other) const {
    return (_value < other.value);
}
bool meters::operator<=(const meters& other) const {
    return (_value <= other.value);
}
bool meters::operator>(const meters& other) const {
    return (_value > other.value);
}
bool meters::operator>=(const meters& other) const {
    return (_value >= other.value);
}
namespace operators {
meters operator+(const meters& A, const meters& B) {
    return meters{A.value + B.value};
}
meters operator-(const meters& A, const meters& B) {
    return meters{A.value - B.value};
}
meters operator*(const meters& A, double factor) {
    return meters{A.value * factor};
}
meters operator*(double factor, const meters& A) {
    return meters{A.value * factor};
}
meters operator/(const meters& A, double factor) {
    return meters{A.value / factor};
}
}  // namespace operators
namespace literals {
iso::meters operator""_m(long double a) {
    return iso::meters(a);
}
iso::meters operator""_m(unsigned long long a) {
    return iso::meters(a);
}
iso::meters operator""_m(const char a[]) {
    return iso::meters(atol(a));
}
}  // namespace literals
}  // namespace iso
