///
/// @file
/// This is an auto-generated source for iso::grams.
/// A measurement of mass
/// @note See README on re-generation.
/// @copyright Copyright 2024.
///
#include "iso/grams.hpp"

namespace iso {
const char* const suffix_type_g::suffix;
// default
grams::grams() : measurement{} {
}
// explicit value
grams::grams(precision a) : measurement{a} {
}
// copy constructor
grams::grams(const grams& other) : measurement{other} {
}
// move constructor
grams::grams(grams&& other) : measurement{std::move(other)} {
}

grams& grams::operator=(const grams &other) noexcept {
    _value = other.value;
    return (*this);
}
grams& grams::operator=(grams& other) noexcept {
    _value = other.value;
    return (*this);
}
grams& grams::operator=(grams&& other) noexcept {
    _value = other.value;
    return (*this);
}
bool grams::operator==(const grams& other) const {
    return basal::equivalent(value, other.value);
}
bool grams::operator!=(const grams& other) const {
    return !operator==(other);
}
grams& grams::operator+=(const grams& other) {
    _value += other.value;
    return (*this);
}
grams& grams::operator-=(const grams& other) {
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
bool grams::operator<(const grams& other) const {
    return (_value < other.value);
}
bool grams::operator<=(const grams& other) const {
    return (_value <= other.value);
}
bool grams::operator>(const grams& other) const {
    return (_value > other.value);
}
bool grams::operator>=(const grams& other) const {
    return (_value >= other.value);
}
namespace operators {
grams operator+(const grams& A, const grams& B) {
    return grams{A.value + B.value};
}
grams operator-(const grams& A, const grams& B) {
    return grams{A.value - B.value};
}
grams operator*(const grams& A, precision factor) {
    return grams{A.value * factor};
}
grams operator*(precision factor, const grams& A) {
    return grams{A.value * factor};
}
grams operator/(const grams& A, precision factor) {
    return grams{A.value / factor};
}
}  // namespace operators
namespace literals {
iso::grams operator""_g(long double a) {
    return iso::grams(static_cast<precision>(a));
}
iso::grams operator""_g(unsigned long long a) {
    return iso::grams(a);
}
iso::grams operator""_g(const char a[]) {
    return iso::grams(atol(a));
}
}  // namespace literals
}  // namespace iso

