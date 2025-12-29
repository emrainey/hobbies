///
/// @file
/// This is an auto-generated source for iso::pascals.
/// A measure of pressure
/// @note See README on re-generation.
/// @copyright Copyright 2025.
///
#include "iso/pascals.hpp"

namespace iso {
char const* const suffix_type_Pa::suffix;
// default
pascals::pascals() : measurement{} {
}
// explicit value
pascals::pascals(precision a) : measurement{a} {
}
// copy constructor
pascals::pascals(pascals const& other) : measurement{other} {
}
// move constructor
pascals::pascals(pascals&& other) : measurement{std::move(other)} {
}

pascals& pascals::operator=(pascals const& other) noexcept {
    _value = other.value;
    return (*this);
}
pascals& pascals::operator=(pascals& other) noexcept {
    _value = other.value;
    return (*this);
}
pascals& pascals::operator=(pascals&& other) noexcept {
    _value = other.value;
    return (*this);
}
bool pascals::operator==(pascals const& other) const {
    return basal::equivalent(value, other.value);
}
bool pascals::operator!=(pascals const& other) const {
    return !operator==(other);
}
pascals& pascals::operator+=(pascals const& other) {
    _value += other.value;
    return (*this);
}
pascals& pascals::operator-=(pascals const& other) {
    _value -= other.value;
    return (*this);
}
pascals& pascals::operator*=(precision factor) {
    _value *= factor;
    return (*this);
}
pascals& pascals::operator/=(precision factor) {
    _value /= factor;
    return (*this);
}
pascals pascals::operator-() const {
    return pascals{-value};
}
bool pascals::operator<(pascals const& other) const {
    return (_value < other.value);
}
bool pascals::operator<=(pascals const& other) const {
    return (_value <= other.value);
}
bool pascals::operator>(pascals const& other) const {
    return (_value > other.value);
}
bool pascals::operator>=(pascals const& other) const {
    return (_value >= other.value);
}
namespace operators {
pascals operator+(pascals const& A, pascals const& B) {
    return pascals{A.value + B.value};
}
pascals operator-(pascals const& A, pascals const& B) {
    return pascals{A.value - B.value};
}
pascals operator*(pascals const& A, precision factor) {
    return pascals{A.value * factor};
}
pascals operator*(precision factor, pascals const& A) {
    return pascals{A.value * factor};
}
pascals operator/(pascals const& A, precision factor) {
    return pascals{A.value / factor};
}
precision operator/(pascals const& A, pascals const& B) {
    return A.value / B.value;
}
}  // namespace operators
namespace literals {
iso::pascals operator""_Pa(long double a) {
    return iso::pascals(static_cast<precision>(a));
}
iso::pascals operator""_Pa(unsigned long long a) {
    return iso::pascals(static_cast<precision>(a));
}
iso::pascals operator""_Pa(char const a[]) {
    iso::precision p = static_cast<iso::precision>(atol(a));
    return iso::pascals(p);
}
}  // namespace literals
}  // namespace iso
