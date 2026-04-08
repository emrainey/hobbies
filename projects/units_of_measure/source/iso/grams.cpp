///
/// @file
/// This is an auto-generated source for iso::grams.
/// A measurement of mass
/// @note See README on re-generation.
/// @copyright Copyright 2026.
///
#include "iso/grams.hpp"

namespace iso {
char const* const suffix_type_g::suffix;
grams::grams() : measurement{} {
}
grams::grams(precision a) : measurement{a} {
}
grams::grams(grams const& other) : measurement{other} {
}
grams::grams(grams&& other) : measurement{std::move(other)} {
}
grams& grams::operator=(grams const& other) noexcept {
    _value = other.value;
    return (*this);
}
grams& grams::operator=(grams& other) noexcept {
    _value = other.value;
    return (*this);
}
grams& grams::operator=(grams&& other) noexcept {
    _value = other.value;
    other._value = static_cast<precision>(0);
    return (*this);
}
bool grams::operator==(grams const& other) const {
    return basal::equivalent(value, other.value);
}
bool grams::operator!=(grams const& other) const {
    return not operator==(other);
}
grams& grams::operator+=(grams const& other) {
    _value += other.value;
    return (*this);
}
grams& grams::operator-=(grams const& other) {
    _value -= other.value;
    return (*this);
}
grams& grams::operator*=(precision factor) {
    _value *= factor;
    return (*this);
}
grams& grams::operator/=(precision factor) {
    _value /= factor;
    return (*this);
}
grams grams::operator-() const {
    return grams{-value};
}
bool grams::operator<(grams const& other) const {
    return (_value < other.value);
}
bool grams::operator<=(grams const& other) const {
    return (_value <= other.value);
}
bool grams::operator>(grams const& other) const {
    return (_value > other.value);
}
bool grams::operator>=(grams const& other) const {
    return (_value >= other.value);
}
namespace operators {
grams operator+(grams const& A, grams const& B) {
    return grams{A.value + B.value};
}
grams operator-(grams const& A, grams const& B) {
    return grams{A.value - B.value};
}
grams operator*(grams const& A, precision factor) {
    return grams{A.value * factor};
}
grams operator*(precision factor, grams const& A) {
    return grams{A.value * factor};
}
grams operator/(grams const& A, precision factor) {
    return grams{A.value / factor};
}
precision operator/(grams const& A, grams const& B) {
    return A.value / B.value;
}
}  // namespace operators
namespace literals {
iso::grams operator""_g(long double a) {
    return iso::grams(static_cast<precision>(a));
}
iso::grams operator""_g(unsigned long long a) {
    return iso::grams(static_cast<precision>(a));
}
iso::grams operator""_g(char const a[]) {
    iso::precision p = static_cast<iso::precision>(atol(a));
    return iso::grams(p);
}
}  // namespace literals
}  // namespace iso

