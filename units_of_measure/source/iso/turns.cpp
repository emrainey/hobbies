/**
 * @file
 * This is an auto-generated source for iso::turns.
 * A measure of an angle
 * @note See README on re-generation.
 * @copyright Copyright 2022.
 */
#include "iso/turns.hpp"

namespace iso {
const char* const suffix_type_Tau::suffix;
// default
turns::turns() : measurement{} {
}
// explicit value
turns::turns(double a) : measurement{a} {
}
// copy constructor
turns::turns(const turns& other) : measurement{other} {
}
// move constructor
turns::turns(turns&& other) : measurement{std::move(other)} {
}

turns& turns::operator=(const turns &other) noexcept {
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
bool turns::operator==(const turns& other) const {
    return iso::equivalent(value, other.value);
}
bool turns::operator!=(const turns& other) const {
    return !operator==(other);
}
turns& turns::operator+=(const turns& other) {
    _value += other.value;
    return (*this);
}
turns& turns::operator-=(const turns& other) {
    _value -= other.value;
    return (*this);
}
turns& turns::operator*=(double factor) {
    _value *= factor;
    return (*this);
}
turns& turns::operator/=(double factor) {
    _value /= factor;
    return (*this);
}
turns turns::operator-() const {
    return turns{-value};
}
bool turns::operator<(const turns& other) const {
    return (_value < other.value);
}
bool turns::operator<=(const turns& other) const {
    return (_value <= other.value);
}
bool turns::operator>(const turns& other) const {
    return (_value > other.value);
}
bool turns::operator>=(const turns& other) const {
    return (_value >= other.value);
}
namespace operators {
turns operator+(const turns& A, const turns& B) {
    return turns{A.value + B.value};
}
turns operator-(const turns& A, const turns& B) {
    return turns{A.value - B.value};
}
turns operator*(const turns& A, double factor) {
    return turns{A.value * factor};
}
turns operator*(double factor, const turns& A) {
    return turns{A.value * factor};
}
turns operator/(const turns& A, double factor) {
    return turns{A.value / factor};
}
}  // namespace operators
namespace literals {
iso::turns operator""_Tau(long double a) {
    return iso::turns(a);
}
iso::turns operator""_Tau(unsigned long long a) {
    return iso::turns(a);
}
iso::turns operator""_Tau(const char a[]) {
    return iso::turns(atol(a));
}
}  // namespace literals
}  // namespace iso

