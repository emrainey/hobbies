/**
 * @file
 * This is an auto-generated source for iso::pascals.
 * 
 * @note See README on re-generation.
 * @copyright Copyright 2022.
 */
#include "iso/pascals.hpp"

namespace iso {
const char *const suffix_type_Pa::suffix;
// default
pascals::pascals() : measurement() {}
// explicit value
pascals::pascals(double a) : measurement(a) {}
// copy constructor
pascals::pascals(const pascals& other) : measurement(other) {}
// move constructor
pascals::pascals(pascals&& other) : measurement(std::move(other)) {}

pascals& pascals::operator=(const pascals &other) noexcept {
    _value = other.value;
    return (*this);
}
pascals& pascals::operator=(pascals &other) noexcept {
    _value = other.value;
    return (*this);
}
pascals& pascals::operator=(pascals &&other) noexcept {
    _value = other.value;
    return (*this);
}
bool pascals::operator==(const pascals &other) const {
    return iso::equivalent(value, other.value);
}
bool pascals::operator!=(const pascals &other) const {
    return !operator==(other);
}
pascals& pascals::operator+=(const pascals& other) {
    _value += other.value;
    return (*this);
}
pascals& pascals::operator-=(const pascals& other) {
    _value -= other.value;
    return (*this);
}
pascals& pascals::operator*=(double factor) {
    _value *= factor;
    return (*this);
}
pascals& pascals::operator/=(double factor) {
    _value /= factor;
    return (*this);
}
pascals pascals::operator-() const {
    return pascals(-value);
}
bool pascals::operator<(const pascals& other) const {
    return (_value < other.value);
}
bool pascals::operator<=(const pascals& other) const {
    return (_value <= other.value);
}
bool pascals::operator>(const pascals& other) const {
    return (_value > other.value);
}
bool pascals::operator>=(const pascals& other) const {
    return (_value >= other.value);
}
namespace operators {
    pascals operator+(const pascals& A, const pascals& B) {
        return pascals(A.value+B.value);
    }
    pascals operator-(const pascals& A, const pascals& B) {
        return pascals(A.value-B.value);
    }
    pascals operator*(const pascals& A, double factor) {
        return pascals(A.value*factor);
    }
    pascals operator*(double factor, const pascals& A) {
        return pascals(A.value*factor);
    }
    pascals operator/(const pascals& A, double factor) {
        return pascals(A.value/factor);
    }
}
namespace literals {
    iso::pascals operator""_Pa(long double a) {
        return iso::pascals(a);
    }
    iso::pascals operator""_Pa(unsigned long long a) {
        return iso::pascals(a);
    }
    iso::pascals operator""_Pa(const char a[]) {
        return iso::pascals(atol(a));
    }
} // namespace literals
} // namespace iso

