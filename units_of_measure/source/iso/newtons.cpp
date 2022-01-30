/**
 * @file
 * This is an auto-generated source for iso::newtons.
 * 
 * @note See README on re-generation.
 * @copyright Copyright 2022.
 */
#include "iso/newtons.hpp"

namespace iso {
const char *const suffix_type_N::suffix;
// default
newtons::newtons() : measurement() {}
// explicit value
newtons::newtons(double a) : measurement(a) {}
// copy constructor
newtons::newtons(const newtons& other) : measurement(other) {}
// move constructor
newtons::newtons(newtons&& other) : measurement(std::move(other)) {}

newtons& newtons::operator=(const newtons &other) noexcept {
    _value = other.value;
    return (*this);
}
newtons& newtons::operator=(newtons &other) noexcept {
    _value = other.value;
    return (*this);
}
newtons& newtons::operator=(newtons &&other) noexcept {
    _value = other.value;
    return (*this);
}
bool newtons::operator==(const newtons &other) const {
    return iso::equivalent(value, other.value);
}
bool newtons::operator!=(const newtons &other) const {
    return !operator==(other);
}
newtons& newtons::operator+=(const newtons& other) {
    _value += other.value;
    return (*this);
}
newtons& newtons::operator-=(const newtons& other) {
    _value -= other.value;
    return (*this);
}
newtons& newtons::operator*=(double factor) {
    _value *= factor;
    return (*this);
}
newtons& newtons::operator/=(double factor) {
    _value /= factor;
    return (*this);
}
newtons newtons::operator-() const {
    return newtons(-value);
}
bool newtons::operator<(const newtons& other) const {
    return (_value < other.value);
}
bool newtons::operator<=(const newtons& other) const {
    return (_value <= other.value);
}
bool newtons::operator>(const newtons& other) const {
    return (_value > other.value);
}
bool newtons::operator>=(const newtons& other) const {
    return (_value >= other.value);
}
namespace operators {
    newtons operator+(const newtons& A, const newtons& B) {
        return newtons(A.value+B.value);
    }
    newtons operator-(const newtons& A, const newtons& B) {
        return newtons(A.value-B.value);
    }
    newtons operator*(const newtons& A, double factor) {
        return newtons(A.value*factor);
    }
    newtons operator*(double factor, const newtons& A) {
        return newtons(A.value*factor);
    }
    newtons operator/(const newtons& A, double factor) {
        return newtons(A.value/factor);
    }
}
namespace literals {
    iso::newtons operator""_N(long double a) {
        return iso::newtons(a);
    }
    iso::newtons operator""_N(unsigned long long a) {
        return iso::newtons(a);
    }
    iso::newtons operator""_N(const char a[]) {
        return iso::newtons(atol(a));
    }
} // namespace literals
} // namespace iso

