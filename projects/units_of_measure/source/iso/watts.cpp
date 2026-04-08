///
/// @file
/// This is an auto-generated source for iso::watts.
/// A measure of electrical power
/// @note See README on re-generation.
/// @copyright Copyright 2026.
///
#include "iso/watts.hpp"

namespace iso {
char const* const suffix_type_W::suffix;
watts::watts() : measurement{} {
}
watts::watts(precision a) : measurement{a} {
}
watts::watts(watts const& other) : measurement{other} {
}
watts::watts(watts&& other) : measurement{std::move(other)} {
}
watts& watts::operator=(watts const& other) noexcept {
    _value = other.value;
    return (*this);
}
watts& watts::operator=(watts& other) noexcept {
    _value = other.value;
    return (*this);
}
watts& watts::operator=(watts&& other) noexcept {
    _value = other.value;
    other._value = static_cast<precision>(0);
    return (*this);
}
bool watts::operator==(watts const& other) const {
    return basal::equivalent(value, other.value);
}
bool watts::operator!=(watts const& other) const {
    return not operator==(other);
}
watts& watts::operator+=(watts const& other) {
    _value += other.value;
    return (*this);
}
watts& watts::operator-=(watts const& other) {
    _value -= other.value;
    return (*this);
}
watts& watts::operator*=(precision factor) {
    _value *= factor;
    return (*this);
}
watts& watts::operator/=(precision factor) {
    _value /= factor;
    return (*this);
}
watts watts::operator-() const {
    return watts{-value};
}
bool watts::operator<(watts const& other) const {
    return (_value < other.value);
}
bool watts::operator<=(watts const& other) const {
    return (_value <= other.value);
}
bool watts::operator>(watts const& other) const {
    return (_value > other.value);
}
bool watts::operator>=(watts const& other) const {
    return (_value >= other.value);
}
namespace operators {
watts operator+(watts const& A, watts const& B) {
    return watts{A.value + B.value};
}
watts operator-(watts const& A, watts const& B) {
    return watts{A.value - B.value};
}
watts operator*(watts const& A, precision factor) {
    return watts{A.value * factor};
}
watts operator*(precision factor, watts const& A) {
    return watts{A.value * factor};
}
watts operator/(watts const& A, precision factor) {
    return watts{A.value / factor};
}
precision operator/(watts const& A, watts const& B) {
    return A.value / B.value;
}
}  // namespace operators
namespace literals {
iso::watts operator""_W(long double a) {
    return iso::watts(static_cast<precision>(a));
}
iso::watts operator""_W(unsigned long long a) {
    return iso::watts(static_cast<precision>(a));
}
iso::watts operator""_W(char const a[]) {
    iso::precision p = static_cast<iso::precision>(atol(a));
    return iso::watts(p);
}
}  // namespace literals
}  // namespace iso

