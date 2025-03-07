///
/// @file
/// This is an auto-generated source for iso::seconds.
/// A measure of time
/// @note See README on re-generation.
/// @copyright Copyright 2025.
///
#include "iso/seconds.hpp"

namespace iso {
char const* const suffix_type_sec::suffix;
// default
seconds::seconds() : measurement{} {
}
// explicit value
seconds::seconds(precision a) : measurement{a} {
}
// copy constructor
seconds::seconds(seconds const& other) : measurement{other} {
}
// move constructor
seconds::seconds(seconds&& other) : measurement{std::move(other)} {
}

seconds& seconds::operator=(seconds const& other) noexcept {
    _value = other.value;
    return (*this);
}
seconds& seconds::operator=(seconds& other) noexcept {
    _value = other.value;
    return (*this);
}
seconds& seconds::operator=(seconds&& other) noexcept {
    _value = other.value;
    return (*this);
}
bool seconds::operator==(seconds const& other) const {
    return basal::equivalent(value, other.value);
}
bool seconds::operator!=(seconds const& other) const {
    return !operator==(other);
}
seconds& seconds::operator+=(seconds const& other) {
    _value += other.value;
    return (*this);
}
seconds& seconds::operator-=(seconds const& other) {
    _value -= other.value;
    return (*this);
}
seconds& seconds::operator*=(precision factor) {
    _value *= factor;
    return (*this);
}
seconds& seconds::operator/=(precision factor) {
    _value /= factor;
    return (*this);
}
seconds seconds::operator-() const {
    return seconds{-value};
}
bool seconds::operator<(seconds const& other) const {
    return (_value < other.value);
}
bool seconds::operator<=(seconds const& other) const {
    return (_value <= other.value);
}
bool seconds::operator>(seconds const& other) const {
    return (_value > other.value);
}
bool seconds::operator>=(seconds const& other) const {
    return (_value >= other.value);
}
namespace operators {
seconds operator+(seconds const& A, seconds const& B) {
    return seconds{A.value + B.value};
}
seconds operator-(seconds const& A, seconds const& B) {
    return seconds{A.value - B.value};
}
seconds operator*(seconds const& A, precision factor) {
    return seconds{A.value * factor};
}
seconds operator*(precision factor, seconds const& A) {
    return seconds{A.value * factor};
}
seconds operator/(seconds const& A, precision factor) {
    return seconds{A.value / factor};
}
precision operator/(seconds const& A, seconds const& B) {
    return A.value / B.value;
}
}  // namespace operators
namespace literals {
iso::seconds operator""_sec(long double a) {
    return iso::seconds(static_cast<precision>(a));
}
iso::seconds operator""_sec(unsigned long long a) {
    return iso::seconds(a);
}
iso::seconds operator""_sec(char const a[]) {
    return iso::seconds(atol(a));
}
}  // namespace literals
}  // namespace iso
