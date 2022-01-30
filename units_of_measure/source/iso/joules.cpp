/**
 * @file
 * This is an auto-generated source for iso::joules.
 * 
 * @note See README on re-generation.
 * @copyright Copyright 2022.
 */
#include "iso/joules.hpp"

namespace iso {
const char *const suffix_type_J::suffix;
// default
joules::joules() : measurement() {}
// explicit value
joules::joules(double a) : measurement(a) {}
// copy constructor
joules::joules(const joules& other) : measurement(other) {}
// move constructor
joules::joules(joules&& other) : measurement(std::move(other)) {}

joules& joules::operator=(const joules &other) noexcept {
    _value = other.value;
    return (*this);
}
joules& joules::operator=(joules &other) noexcept {
    _value = other.value;
    return (*this);
}
joules& joules::operator=(joules &&other) noexcept {
    _value = other.value;
    return (*this);
}
bool joules::operator==(const joules &other) const {
    return iso::equivalent(value, other.value);
}
bool joules::operator!=(const joules &other) const {
    return !operator==(other);
}
joules& joules::operator+=(const joules& other) {
    _value += other.value;
    return (*this);
}
joules& joules::operator-=(const joules& other) {
    _value -= other.value;
    return (*this);
}
joules& joules::operator*=(double factor) {
    _value *= factor;
    return (*this);
}
joules& joules::operator/=(double factor) {
    _value /= factor;
    return (*this);
}
joules joules::operator-() const {
    return joules(-value);
}
bool joules::operator<(const joules& other) const {
    return (_value < other.value);
}
bool joules::operator<=(const joules& other) const {
    return (_value <= other.value);
}
bool joules::operator>(const joules& other) const {
    return (_value > other.value);
}
bool joules::operator>=(const joules& other) const {
    return (_value >= other.value);
}
namespace operators {
    joules operator+(const joules& A, const joules& B) {
        return joules(A.value+B.value);
    }
    joules operator-(const joules& A, const joules& B) {
        return joules(A.value-B.value);
    }
    joules operator*(const joules& A, double factor) {
        return joules(A.value*factor);
    }
    joules operator*(double factor, const joules& A) {
        return joules(A.value*factor);
    }
    joules operator/(const joules& A, double factor) {
        return joules(A.value/factor);
    }
}
namespace literals {
    iso::joules operator""_J(long double a) {
        return iso::joules(a);
    }
    iso::joules operator""_J(unsigned long long a) {
        return iso::joules(a);
    }
    iso::joules operator""_J(const char a[]) {
        return iso::joules(atol(a));
    }
} // namespace literals
} // namespace iso

