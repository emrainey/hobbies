/**
 * @file
 * This is an auto-generated source for iso::amperes.
 *
 * @note See README on re-generation.
 * @copyright Copyright 2022.
 */
#include "iso/amperes.hpp"

namespace iso {
const char* const suffix_type_A::suffix;
// default
amperes::amperes() : measurement() {
}
// explicit value
amperes::amperes(double a) : measurement(a) {
}
// copy constructor
amperes::amperes(const amperes& other) : measurement(other) {
}
// move constructor
amperes::amperes(amperes&& other) : measurement(std::move(other)) {
}

amperes& amperes::operator=(const amperes& other) noexcept {
    _value = other.value;
    return (*this);
}
amperes& amperes::operator=(amperes& other) noexcept {
    _value = other.value;
    return (*this);
}
amperes& amperes::operator=(amperes&& other) noexcept {
    _value = other.value;
    return (*this);
}
bool amperes::operator==(const amperes& other) const {
    return iso::equivalent(value, other.value);
}
bool amperes::operator!=(const amperes& other) const {
    return !operator==(other);
}
amperes& amperes::operator+=(const amperes& other) {
    _value += other.value;
    return (*this);
}
amperes& amperes::operator-=(const amperes& other) {
    _value -= other.value;
    return (*this);
}
amperes& amperes::operator*=(double factor) {
    _value *= factor;
    return (*this);
}
amperes& amperes::operator/=(double factor) {
    _value /= factor;
    return (*this);
}
amperes amperes::operator-() const {
    return amperes(-value);
}
bool amperes::operator<(const amperes& other) const {
    return (_value < other.value);
}
bool amperes::operator<=(const amperes& other) const {
    return (_value <= other.value);
}
bool amperes::operator>(const amperes& other) const {
    return (_value > other.value);
}
bool amperes::operator>=(const amperes& other) const {
    return (_value >= other.value);
}
namespace operators {
amperes operator+(const amperes& A, const amperes& B) {
    return amperes(A.value + B.value);
}
amperes operator-(const amperes& A, const amperes& B) {
    return amperes(A.value - B.value);
}
amperes operator*(const amperes& A, double factor) {
    return amperes(A.value * factor);
}
amperes operator*(double factor, const amperes& A) {
    return amperes(A.value * factor);
}
amperes operator/(const amperes& A, double factor) {
    return amperes(A.value / factor);
}
}  // namespace operators
namespace literals {
iso::amperes operator""_A(long double a) {
    return iso::amperes(a);
}
iso::amperes operator""_A(unsigned long long a) {
    return iso::amperes(a);
}
iso::amperes operator""_A(const char a[]) {
    return iso::amperes(atol(a));
}
}  // namespace literals
}  // namespace iso
