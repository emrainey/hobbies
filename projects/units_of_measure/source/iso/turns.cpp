///
/// @file
/// This is an auto-generated source for iso::turns.
/// A measure of an angle
/// @note See README on re-generation.
/// @copyright Copyright 2025.
///
#include "iso/turns.hpp"

namespace iso {
char const* const suffix_type_Tau::suffix;
// default
turns::turns() : measurement{} {
}
// explicit value
turns::turns(precision a) : measurement{a} {
}
// copy constructor
turns::turns(turns const& other) : measurement{other} {
}
// move constructor
turns::turns(turns&& other) : measurement{std::move(other)} {
}

turns& turns::operator=(turns const& other) noexcept {
    _value = other.value;
    return (*this);
}
turns& turns::operator=(turns& other) noexcept {
    _value = other.value;
    return (*this);
}
turns& turns::operator=(turns&& other) noexcept {
    _value = other.value;
    return (*this);
}
bool turns::operator==(turns const& other) const {
    return basal::equivalent(value, other.value);
}
bool turns::operator!=(turns const& other) const {
    return !operator==(other);
}
turns& turns::operator+=(turns const& other) {
    _value += other.value;
    return (*this);
}
turns& turns::operator-=(turns const& other) {
    _value -= other.value;
    return (*this);
}
turns& turns::operator*=(precision factor) {
    _value *= factor;
    return (*this);
}
turns& turns::operator/=(precision factor) {
    _value /= factor;
    return (*this);
}
turns turns::operator-() const {
    return turns{-value};
}
bool turns::operator<(turns const& other) const {
    return (_value < other.value);
}
bool turns::operator<=(turns const& other) const {
    return (_value <= other.value);
}
bool turns::operator>(turns const& other) const {
    return (_value > other.value);
}
bool turns::operator>=(turns const& other) const {
    return (_value >= other.value);
}
namespace operators {
turns operator+(turns const& A, turns const& B) {
    return turns{A.value + B.value};
}
turns operator-(turns const& A, turns const& B) {
    return turns{A.value - B.value};
}
turns operator*(turns const& A, precision factor) {
    return turns{A.value * factor};
}
turns operator*(precision factor, turns const& A) {
    return turns{A.value * factor};
}
turns operator/(turns const& A, precision factor) {
    return turns{A.value / factor};
}
precision operator/(turns const& A, turns const& B) {
    return A.value / B.value;
}
}  // namespace operators
namespace literals {
iso::turns operator""_Tau(long double a) {
    return iso::turns(static_cast<precision>(a));
}
iso::turns operator""_Tau(unsigned long long a) {
    return iso::turns(static_cast<precision>(a));
}
iso::turns operator""_Tau(char const a[]) {
    iso::precision p = static_cast<iso::precision>(atol(a));
    return iso::turns(p);
}
}  // namespace literals
}  // namespace iso
