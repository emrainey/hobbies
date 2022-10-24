/**
 * @file
 * This is an auto-generated source for iso::ohms.
 * A measure of resistance to current
 * @note See README on re-generation.
 * @copyright Copyright 2022.
 */
#include "iso/ohms.hpp"

namespace iso {
const char* const suffix_type_Ohm::suffix;
// default
ohms::ohms() : measurement{} {
}
// explicit value
ohms::ohms(double a) : measurement{a} {
}
// copy constructor
ohms::ohms(const ohms& other) : measurement{other} {
}
// move constructor
ohms::ohms(ohms&& other) : measurement{std::move(other)} {
}

ohms& ohms::operator=(const ohms &other) noexcept {
    _value = other.value;
    return (*this);
}
ohms& ohms::operator=(ohms& other) noexcept {
    _value = other.value;
    return (*this);
}
ohms& ohms::operator=(ohms&& other) noexcept {
    _value = other.value;
    return (*this);
}
bool ohms::operator==(const ohms& other) const {
    return iso::equivalent(value, other.value);
}
bool ohms::operator!=(const ohms& other) const {
    return !operator==(other);
}
ohms& ohms::operator+=(const ohms& other) {
    _value += other.value;
    return (*this);
}
ohms& ohms::operator-=(const ohms& other) {
    _value -= other.value;
    return (*this);
}
ohms& ohms::operator*=(double factor) {
    _value *= factor;
    return (*this);
}
ohms& ohms::operator/=(double factor) {
    _value /= factor;
    return (*this);
}
ohms ohms::operator-() const {
    return ohms{-value};
}
bool ohms::operator<(const ohms& other) const {
    return (_value < other.value);
}
bool ohms::operator<=(const ohms& other) const {
    return (_value <= other.value);
}
bool ohms::operator>(const ohms& other) const {
    return (_value > other.value);
}
bool ohms::operator>=(const ohms& other) const {
    return (_value >= other.value);
}
namespace operators {
ohms operator+(const ohms& A, const ohms& B) {
    return ohms{A.value + B.value};
}
ohms operator-(const ohms& A, const ohms& B) {
    return ohms{A.value - B.value};
}
ohms operator*(const ohms& A, double factor) {
    return ohms{A.value * factor};
}
ohms operator*(double factor, const ohms& A) {
    return ohms{A.value * factor};
}
ohms operator/(const ohms& A, double factor) {
    return ohms{A.value / factor};
}
}  // namespace operators
namespace literals {
iso::ohms operator""_Ohm(long double a) {
    return iso::ohms(a);
}
iso::ohms operator""_Ohm(unsigned long long a) {
    return iso::ohms(a);
}
iso::ohms operator""_Ohm(const char a[]) {
    return iso::ohms(atol(a));
}
}  // namespace literals
}  // namespace iso

