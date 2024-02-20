///
/// @file
/// This is an auto-generated source for iso::meters.
/// A measure of distance
/// @note See README on re-generation.
/// @copyright Copyright 2024.
///
#include "iso/meters.hpp"

namespace iso {
char const* const suffix_type_m::suffix;
// default
meters::meters() : measurement{} {
}
// explicit value
meters::meters(precision a) : measurement{a} {
}
// copy constructor
meters::meters(meters const& other) : measurement{other} {
}
// move constructor
meters::meters(meters&& other) : measurement{std::move(other)} {
}

meters& meters::operator=(const meters &other) noexcept {
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
bool meters::operator==(meters const& other) const {
    return basal::equivalent(value, other.value);
}
bool meters::operator!=(meters const& other) const {
    return !operator==(other);
}
meters& meters::operator+=(meters const& other) {
    _value += other.value;
    return (*this);
}
meters& meters::operator-=(meters const& other) {
    _value -= other.value;
    return (*this);
}
meters& meters::operator*=(precision factor) {
    _value *= factor;
    return (*this);
}
meters& meters::operator/=(precision factor) {
    _value /= factor;
    return (*this);
}
meters meters::operator-() const {
    return meters{-value};
}
bool meters::operator<(meters const& other) const {
    return (_value < other.value);
}
bool meters::operator<=(meters const& other) const {
    return (_value <= other.value);
}
bool meters::operator>(meters const& other) const {
    return (_value > other.value);
}
bool meters::operator>=(meters const& other) const {
    return (_value >= other.value);
}
namespace operators {
meters operator+(meters const& A, meters const& B) {
    return meters{A.value + B.value};
}
meters operator-(meters const& A, meters const& B) {
    return meters{A.value - B.value};
}
meters operator*(meters const& A, precision factor) {
    return meters{A.value * factor};
}
meters operator*(precision factor, meters const& A) {
    return meters{A.value * factor};
}
meters operator/(meters const& A, precision factor) {
    return meters{A.value / factor};
}
}  // namespace operators
namespace literals {
iso::meters operator""_m(long double a) {
    return iso::meters(static_cast<precision>(a));
}
iso::meters operator""_m(unsigned long long a) {
    return iso::meters(a);
}
iso::meters operator""_m(char const a[]) {
    return iso::meters(atol(a));
}
}  // namespace literals
}  // namespace iso

