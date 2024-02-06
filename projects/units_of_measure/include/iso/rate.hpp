#pragma once

#include <cstring>

#include "iso/measurement.hpp"

namespace iso {
// How to handle compound rate measurements?
// Rate template!
template <typename NumeratorType, typename DenominatorType>
class rate {
    static_assert(not std::is_same<NumeratorType, DenominatorType>::value, "Must be different types!");
    static_assert(std::is_same<typename NumeratorType::type, typename DenominatorType::type>::value,
                  "Unit types must be the same!");

public:
    using type = typename NumeratorType::type;

protected:
    /// The simplified value
    type _value;

public:
    const type& value;

protected:
    NumeratorType _numerator;
    DenominatorType _denominator;

public:
    const NumeratorType& numerator;
    const DenominatorType& denominator;

protected:
    char suffix[12];

public:
    /// Default Constructor
    rate()
        : _value{0}
        , value{_value}
        , _numerator{0}
        , _denominator{1}
        , numerator{_numerator}
        , denominator{_denominator} {
        snprintf(suffix, sizeof(suffix), "%s/%s", _numerator.get_suffix(), _denominator.get_suffix());
    }

    rate(const NumeratorType& num, const DenominatorType& denom)
        : _value{0}
        , value{_value}
        , _numerator{num}
        , _denominator{denom}
        , numerator{_numerator}
        , denominator{_denominator} {
        _value = reduce();
        snprintf(suffix, sizeof(suffix), "%s/%s", _numerator.get_suffix(), _denominator.get_suffix());
    }

    rate(double num, double denom)
        : _value{0}
        , value{_value}
        , _numerator{num}
        , _denominator{denom}
        , numerator{_numerator}
        , denominator{_denominator} {
        _value = reduce();
        snprintf(suffix, sizeof(suffix), "%s/%s", _numerator.get_suffix(), _denominator.get_suffix());
    }

    rate(const rate& other)
        : _value{other.value}
        , value{_value}
        , _numerator{other.numerator}
        , _denominator{other.denominator}
        , numerator{_numerator}
        , denominator{_denominator} {
        std::strncpy(suffix, other.suffix, sizeof(suffix));
        suffix[sizeof(suffix) - 1] = '\0';
    }

    rate& operator=(const rate& other) {
        std::strncpy(suffix, other.suffix, sizeof(suffix));
        suffix[sizeof(suffix) - 1] = '\0';
        _value = other.value;
        _numerator = other._numerator;
        _denominator = other._denominator;
        return (*this);
    }

    virtual ~rate() {
    }

    typename NumeratorType::type reduce() const {
        return _numerator.reduce() / _denominator.reduce();
    }

    type operator()() const {
        return _value;
    }

    bool operator==(const rate& other) {
        return basal::equivalent(_value, other.value);
    }

    bool operator!=(const rate& other) {
        return !operator==(other);
    }

    bool operator<(const rate& other) {
        return (_value < other.value);
    }

    bool operator>(const rate& other) {
        return (_value > other.value);
    }

    bool operator<=(const rate& other) {
        return (_value <= other.value);
    }

    bool operator>=(const rate& other) {
        return (_value >= other.value);
    }

    const char* get_suffix() const {
        return suffix;
    }
};
namespace streams {
template <typename NumeratorType, typename DenominatorType>
std::ostream& operator<<(std::ostream& output, rate<NumeratorType, DenominatorType>& ref) {
    return output << ref.numerator.value << '/' << ref.denominator.value << " " << ref.numerator.get_suffix() << '/'
                  << ref.denominator.get_suffix() << std::endl;
}
}  // namespace streams
}  // namespace iso
