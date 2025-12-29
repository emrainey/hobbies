///
/// @file
/// This is an auto-generated source for iso::ohms.
/// A measure of resistance to current
/// @note See README on re-generation.
/// @copyright Copyright 2025.
///
#include "iso/ohms.hpp"

namespace iso {
char const* const suffix_type_Ohm::suffix;
// default
ohms::ohms() : measurement{} {
}
// explicit value
ohms::ohms(precision a) : measurement{a} {
}
// copy constructor
ohms::ohms(ohms const& other) : measurement{other} {
}
// move constructor
ohms::ohms(ohms&& other) : measurement{std::move(other)} {
}

ohms& ohms::operator=(ohms const& other) noexcept {
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
bool ohms::operator==(ohms const& other) const {
    return basal::equivalent(value, other.value);
}
bool ohms::operator!=(ohms const& other) const {
    return !operator==(other);
}
ohms& ohms::operator+=(ohms const& other) {
    _value += other.value;
    return (*this);
}
ohms& ohms::operator-=(ohms const& other) {
    _value -= other.value;
    return (*this);
}
ohms& ohms::operator*=(precision factor) {
    _value *= factor;
    return (*this);
}
ohms& ohms::operator/=(precision factor) {
    _value /= factor;
    return (*this);
}
ohms ohms::operator-() const {
    return ohms{-value};
}
bool ohms::operator<(ohms const& other) const {
    return (_value < other.value);
}
bool ohms::operator<=(ohms const& other) const {
    return (_value <= other.value);
}
bool ohms::operator>(ohms const& other) const {
    return (_value > other.value);
}
bool ohms::operator>=(ohms const& other) const {
    return (_value >= other.value);
}
namespace operators {
ohms operator+(ohms const& A, ohms const& B) {
    return ohms{A.value + B.value};
}
ohms operator-(ohms const& A, ohms const& B) {
    return ohms{A.value - B.value};
}
ohms operator*(ohms const& A, precision factor) {
    return ohms{A.value * factor};
}
ohms operator*(precision factor, ohms const& A) {
    return ohms{A.value * factor};
}
ohms operator/(ohms const& A, precision factor) {
    return ohms{A.value / factor};
}
precision operator/(ohms const& A, ohms const& B) {
    return A.value / B.value;
}
}  // namespace operators
namespace literals {
iso::ohms operator""_Ohm(long double a) {
    return iso::ohms(static_cast<precision>(a));
}
iso::ohms operator""_Ohm(unsigned long long a) {
    return iso::ohms(static_cast<precision>(a));
}
iso::ohms operator""_Ohm(char const a[]) {
    iso::precision p = static_cast<iso::precision>(atol(a));
    return iso::ohms(p);
}
}  // namespace literals
}  // namespace iso
