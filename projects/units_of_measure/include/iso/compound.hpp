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
    /// The simplified value
    type _value;

public:
    /// The read-only simplified value
    type const& value;

protected:
    LeftType _first;
    RightType _second;

public:
    LeftType const& first;
    RightType const& second;

protected:
    char suffix[10];

public:
    compound() : _value{0}, value{_value}, _first{0}, _second{0}, first{_first}, second{_second} {
        snprintf(suffix, sizeof(suffix), "%s%s", _first.get_suffix(), _second.get_suffix());
    }

    compound(long double left, long double rght)
        : _value{left * rght}, value{_value}, _first{left}, _second{rght}, first{_first}, second{_second} {
        snprintf(suffix, sizeof(suffix), "%s%s", _first.get_suffix(), _second.get_suffix());
    }

    compound(LeftType const& f, RightType const& s)
        : _value{0}, value{_value}, _first{f}, _second{s}, first{_first}, second{_second} {
        _value = reduce();
        snprintf(suffix, sizeof(suffix), "%s%s", _first.get_suffix(), _second.get_suffix());
    }

    compound& operator=(compound const& other) {
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

    bool operator==(compound const& other) {
        return equivalent(_value, other.value);
    }
    bool operator!=(compound const& other) {
        return !operator==(other);
    }

    bool operator<(compound const& other) {
        return (_value < other.value);
    }

    bool operator>(compound const& other) {
        return (_value > other.value);
    }

    bool operator<=(compound const& other) {
        return (_value <= other.value);
    }

    bool operator>=(compound const& other) {
        return (_value >= other.value);
    }

    char const* get_suffix() const {
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
