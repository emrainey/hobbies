///
/// @file
/// This is an auto-generated source for iso::amperes.
/// A measure of current in electricity
/// @note See README on re-generation.
/// @copyright Copyright 2025.
///
#include "iso/amperes.hpp"

namespace iso {
char const* const suffix_type_A::suffix;
// default
amperes::amperes() : measurement{} {
}
// explicit value
amperes::amperes(precision a) : measurement{a} {
}
// copy constructor
amperes::amperes(amperes const& other) : measurement{other} {
}
// move constructor
amperes::amperes(amperes&& other) : measurement{std::move(other)} {
}

amperes& amperes::operator=(amperes const& other) noexcept {
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
bool amperes::operator==(amperes const& other) const {
    return basal::equivalent(value, other.value);
}
bool amperes::operator!=(amperes const& other) const {
    return !operator==(other);
}
amperes& amperes::operator+=(amperes const& other) {
    _value += other.value;
    return (*this);
}
amperes& amperes::operator-=(amperes const& other) {
    _value -= other.value;
    return (*this);
}
amperes& amperes::operator*=(precision factor) {
    _value *= factor;
    return (*this);
}
amperes& amperes::operator/=(precision factor) {
    _value /= factor;
    return (*this);
}
amperes amperes::operator-() const {
    return amperes{-value};
}
bool amperes::operator<(amperes const& other) const {
    return (_value < other.value);
}
bool amperes::operator<=(amperes const& other) const {
    return (_value <= other.value);
}
bool amperes::operator>(amperes const& other) const {
    return (_value > other.value);
}
bool amperes::operator>=(amperes const& other) const {
    return (_value >= other.value);
}
namespace operators {
amperes operator+(amperes const& A, amperes const& B) {
    return amperes{A.value + B.value};
}
amperes operator-(amperes const& A, amperes const& B) {
    return amperes{A.value - B.value};
}
amperes operator*(amperes const& A, precision factor) {
    return amperes{A.value * factor};
}
amperes operator*(precision factor, amperes const& A) {
    return amperes{A.value * factor};
}
amperes operator/(amperes const& A, precision factor) {
    return amperes{A.value / factor};
}
precision operator/(amperes const& A, amperes const& B) {
    return A.value / B.value;
}
}  // namespace operators
namespace literals {
iso::amperes operator""_A(long double a) {
    return iso::amperes(static_cast<precision>(a));
}
iso::amperes operator""_A(unsigned long long a) {
    return iso::amperes(a);
}
iso::amperes operator""_A(char const a[]) {
    return iso::amperes(atol(a));
}
}  // namespace literals
}  // namespace iso
