/**
 * @file
 * This is an auto-generated source for iso::seconds.
 * 
 * @note See README on re-generation.
 * @copyright Copyright 2022.
 */
#include "iso/seconds.hpp"

namespace iso {
const char *const suffix_type_sec::suffix;
// default
seconds::seconds() : measurement() {}
// explicit value
seconds::seconds(double a) : measurement(a) {}
// copy constructor
seconds::seconds(const seconds& other) : measurement(other) {}
// move constructor
seconds::seconds(seconds&& other) : measurement(std::move(other)) {}

seconds& seconds::operator=(const seconds &other) noexcept {
    _value = other.value;
    return (*this);
}
seconds& seconds::operator=(seconds &other) noexcept {
    _value = other.value;
    return (*this);
}
seconds& seconds::operator=(seconds &&other) noexcept {
    _value = other.value;
    return (*this);
}
bool seconds::operator==(const seconds &other) const {
    return iso::equivalent(value, other.value);
}
bool seconds::operator!=(const seconds &other) const {
    return !operator==(other);
}
seconds& seconds::operator+=(const seconds& other) {
    _value += other.value;
    return (*this);
}
seconds& seconds::operator-=(const seconds& other) {
    _value -= other.value;
    return (*this);
}
seconds& seconds::operator*=(double factor) {
    _value *= factor;
    return (*this);
}
seconds& seconds::operator/=(double factor) {
    _value /= factor;
    return (*this);
}
seconds seconds::operator-() const {
    return seconds(-value);
}
bool seconds::operator<(const seconds& other) const {
    return (_value < other.value);
}
bool seconds::operator<=(const seconds& other) const {
    return (_value <= other.value);
}
bool seconds::operator>(const seconds& other) const {
    return (_value > other.value);
}
bool seconds::operator>=(const seconds& other) const {
    return (_value >= other.value);
}
namespace operators {
    seconds operator+(const seconds& A, const seconds& B) {
        return seconds(A.value+B.value);
    }
    seconds operator-(const seconds& A, const seconds& B) {
        return seconds(A.value-B.value);
    }
    seconds operator*(const seconds& A, double factor) {
        return seconds(A.value*factor);
    }
    seconds operator*(double factor, const seconds& A) {
        return seconds(A.value*factor);
    }
    seconds operator/(const seconds& A, double factor) {
        return seconds(A.value/factor);
    }
}
namespace literals {
    iso::seconds operator""_sec(long double a) {
        return iso::seconds(a);
    }
    iso::seconds operator""_sec(unsigned long long a) {
        return iso::seconds(a);
    }
    iso::seconds operator""_sec(const char a[]) {
        return iso::seconds(atol(a));
    }
} // namespace literals
} // namespace iso

