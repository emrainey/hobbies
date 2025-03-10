///
/// @file
/// This is an auto-generated source for iso::joules.
/// A measure of energy
/// @note See README on re-generation.
/// @copyright Copyright 2025.
///
#include "iso/joules.hpp"

namespace iso {
char const* const suffix_type_J::suffix;
// default
joules::joules() : measurement{} {
}
// explicit value
joules::joules(precision a) : measurement{a} {
}
// copy constructor
joules::joules(joules const& other) : measurement{other} {
}
// move constructor
joules::joules(joules&& other) : measurement{std::move(other)} {
}

joules& joules::operator=(joules const& other) noexcept {
    _value = other.value;
    return (*this);
}
joules& joules::operator=(joules& other) noexcept {
    _value = other.value;
    return (*this);
}
joules& joules::operator=(joules&& other) noexcept {
    _value = other.value;
    return (*this);
}
bool joules::operator==(joules const& other) const {
    return basal::equivalent(value, other.value);
}
bool joules::operator!=(joules const& other) const {
    return !operator==(other);
}
joules& joules::operator+=(joules const& other) {
    _value += other.value;
    return (*this);
}
joules& joules::operator-=(joules const& other) {
    _value -= other.value;
    return (*this);
}
joules& joules::operator*=(precision factor) {
    _value *= factor;
    return (*this);
}
joules& joules::operator/=(precision factor) {
    _value /= factor;
    return (*this);
}
joules joules::operator-() const {
    return joules{-value};
}
bool joules::operator<(joules const& other) const {
    return (_value < other.value);
}
bool joules::operator<=(joules const& other) const {
    return (_value <= other.value);
}
bool joules::operator>(joules const& other) const {
    return (_value > other.value);
}
bool joules::operator>=(joules const& other) const {
    return (_value >= other.value);
}
namespace operators {
joules operator+(joules const& A, joules const& B) {
    return joules{A.value + B.value};
}
joules operator-(joules const& A, joules const& B) {
    return joules{A.value - B.value};
}
joules operator*(joules const& A, precision factor) {
    return joules{A.value * factor};
}
joules operator*(precision factor, joules const& A) {
    return joules{A.value * factor};
}
joules operator/(joules const& A, precision factor) {
    return joules{A.value / factor};
}
precision operator/(joules const& A, joules const& B) {
    return A.value / B.value;
}
}  // namespace operators
namespace literals {
iso::joules operator""_J(long double a) {
    return iso::joules(static_cast<precision>(a));
}
iso::joules operator""_J(unsigned long long a) {
    return iso::joules(static_cast<precision>(a));
}
iso::joules operator""_J(char const a[]) {
    iso::precision p = static_cast<iso::precision>(atol(a));
    return iso::joules(p);
}
}  // namespace literals
}  // namespace iso

