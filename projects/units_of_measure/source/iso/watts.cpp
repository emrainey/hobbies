/**
 * @file
 * This is an auto-generated source for iso::watts.
 * A measure of electrical power
 * @note See README on re-generation.
 * @copyright Copyright 2022.
 */
#include "iso/watts.hpp"

namespace iso {
const char* const suffix_type_W::suffix;
// default
watts::watts() : measurement{} {
}
// explicit value
watts::watts(double a) : measurement{a} {
}
// copy constructor
watts::watts(const watts& other) : measurement{other} {
}
// move constructor
watts::watts(watts&& other) : measurement{std::move(other)} {
}

watts& watts::operator=(const watts &other) noexcept {
    _value = other.value;
    return (*this);
}
watts& watts::operator=(watts& other) noexcept {
    _value = other.value;
    return (*this);
}
watts& watts::operator=(watts&& other) noexcept {
    _value = other.value;
    return (*this);
}
bool watts::operator==(const watts& other) const {
    return iso::equivalent(value, other.value);
}
bool watts::operator!=(const watts& other) const {
    return !operator==(other);
}
watts& watts::operator+=(const watts& other) {
    _value += other.value;
    return (*this);
}
watts& watts::operator-=(const watts& other) {
    _value -= other.value;
    return (*this);
}
watts& watts::operator*=(double factor) {
    _value *= factor;
    return (*this);
}
watts& watts::operator/=(double factor) {
    _value /= factor;
    return (*this);
}
watts watts::operator-() const {
    return watts{-value};
}
bool watts::operator<(const watts& other) const {
    return (_value < other.value);
}
bool watts::operator<=(const watts& other) const {
    return (_value <= other.value);
}
bool watts::operator>(const watts& other) const {
    return (_value > other.value);
}
bool watts::operator>=(const watts& other) const {
    return (_value >= other.value);
}
namespace operators {
watts operator+(const watts& A, const watts& B) {
    return watts{A.value + B.value};
}
watts operator-(const watts& A, const watts& B) {
    return watts{A.value - B.value};
}
watts operator*(const watts& A, double factor) {
    return watts{A.value * factor};
}
watts operator*(double factor, const watts& A) {
    return watts{A.value * factor};
}
watts operator/(const watts& A, double factor) {
    return watts{A.value / factor};
}
}  // namespace operators
namespace literals {
iso::watts operator""_W(long double a) {
    return iso::watts(a);
}
iso::watts operator""_W(unsigned long long a) {
    return iso::watts(a);
}
iso::watts operator""_W(const char a[]) {
    return iso::watts(atol(a));
}
}  // namespace literals
}  // namespace iso

