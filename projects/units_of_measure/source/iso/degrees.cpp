///
/// @file
/// This is an auto-generated source for iso::degrees.
/// A measure of an angle
/// @note See README on re-generation.
/// @copyright Copyright 2025.
///
#include "iso/degrees.hpp"

namespace iso {
char const* const suffix_type_deg::suffix;
// default
degrees::degrees() : measurement{} {
}
// explicit value
degrees::degrees(precision a) : measurement{a} {
}
// copy constructor
degrees::degrees(degrees const& other) : measurement{other} {
}
// move constructor
degrees::degrees(degrees&& other) : measurement{std::move(other)} {
}

degrees& degrees::operator=(degrees const& other) noexcept {
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
bool degrees::operator==(degrees const& other) const {
    return basal::equivalent(value, other.value);
}
bool degrees::operator!=(degrees const& other) const {
    return !operator==(other);
}
degrees& degrees::operator+=(degrees const& other) {
    _value += other.value;
    return (*this);
}
degrees& degrees::operator-=(degrees const& other) {
    _value -= other.value;
    return (*this);
}
degrees& degrees::operator*=(precision factor) {
    _value *= factor;
    return (*this);
}
degrees& degrees::operator/=(precision factor) {
    _value /= factor;
    return (*this);
}
degrees degrees::operator-() const {
    return degrees{-value};
}
bool degrees::operator<(degrees const& other) const {
    return (_value < other.value);
}
bool degrees::operator<=(degrees const& other) const {
    return (_value <= other.value);
}
bool degrees::operator>(degrees const& other) const {
    return (_value > other.value);
}
bool degrees::operator>=(degrees const& other) const {
    return (_value >= other.value);
}
namespace operators {
degrees operator+(degrees const& A, degrees const& B) {
    return degrees{A.value + B.value};
}
degrees operator-(degrees const& A, degrees const& B) {
    return degrees{A.value - B.value};
}
degrees operator*(degrees const& A, precision factor) {
    return degrees{A.value * factor};
}
degrees operator*(precision factor, degrees const& A) {
    return degrees{A.value * factor};
}
degrees operator/(degrees const& A, precision factor) {
    return degrees{A.value / factor};
}
precision operator/(degrees const& A, degrees const& B) {
    return A.value / B.value;
}
}  // namespace operators
namespace literals {
iso::degrees operator""_deg(long double a) {
    return iso::degrees(static_cast<precision>(a));
}
iso::degrees operator""_deg(unsigned long long a) {
    return iso::degrees(static_cast<precision>(a));
}
iso::degrees operator""_deg(char const a[]) {
    iso::precision p = static_cast<iso::precision>(atol(a));
    return iso::degrees(p);
}
}  // namespace literals
}  // namespace iso

