#pragma once

#include "iso/measurement.hpp"

namespace iso {
// How to handle compound (multiplicative) measurements (N*m)

template <class LeftType, class RightType>
class compound {
    static_assert(not std::is_same<LeftType, RightType>::value, "Must be different types!");
    static_assert(std::is_same<typename LeftType::type, typename RightType::type>::value,
                  "Unit types must be the same!");

public:
    using type = typename LeftType::type;

protected:
    /** The simplified value */
    type _value;

public:
    /** The read-only simplified value */
    const type& value;

protected:
    LeftType _first;
    RightType _second;

public:
    const LeftType& first;
    const RightType& second;

protected:
    char suffix[10];

public:
    compound() : _value(0), value(_value), _first(0.0), _second(0.0), first(_first), second(_second) {
        snprintf(suffix, sizeof(suffix), "%s%s", _first.get_suffix(), _second.get_suffix());
    }

    compound(const double left, const double rght)
        : _value{left * rght}, value{_value}, _first{left}, _second{rght}, first{_first}, second{_second} {
        snprintf(suffix, sizeof(suffix), "%s%s", _first.get_suffix(), _second.get_suffix());
    }

    compound(const LeftType& f, const RightType& s)
        : _value{0}, value{_value}, _first{f}, _second{s}, first{_first}, second{_second} {
        _value = reduce();
        snprintf(suffix, sizeof(suffix), "%s%s", _first.get_suffix(), _second.get_suffix());
    }

    compound& operator=(const compound& other) {
        _value = other.value;
        _first = other._first;
        _second = other._second;
        snprintf(suffix, sizeof(suffix), "%s%s", _first.get_suffix(), _second.get_suffix());
        return (*this);
    }

    virtual ~compound() {
    }

    typename LeftType::type reduce() const {
        return _first.reduce() * _second.reduce();
    }

    type operator()() const {
        return _value;
    }

    bool operator==(const compound& other) {
        return equivalent(_value, other.value);
    }
    bool operator!=(const compound& other) {
        return !operator==(other);
    }

    bool operator<(const compound& other) {
        return (_value < other.value);
    }

    bool operator>(const compound& other) {
        return (_value > other.value);
    }

    bool operator<=(const compound& other) {
        return (_value <= other.value);
    }

    bool operator>=(const compound& other) {
        return (_value >= other.value);
    }

    const char* get_suffix() const {
        return suffix;
    }
};

namespace streams {
template <typename LeftType, typename RightType>
std::ostream& operator<<(std::ostream& output, compound<LeftType, RightType>& ref) {
    return output << ref.first.value << '*' << ref.second.value << " " << ref.first.get_suffix() << '*'
                  << ref.second.get_suffix() << std::endl;
}

}  // namespace streams

}  // namespace iso
