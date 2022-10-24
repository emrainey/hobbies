#pragma once

#include <cmath>
#include <ostream>

#include "iec/prefix.hpp"
#include "si/prefix.hpp"

/** The ISO namespace */
namespace iso {

/** A simple float/double equals */
inline bool equivalent(const double a, const double b) {
    return not(a > b) and not(a < b);
}

/**
 * Converts the value to another scale.
 */
template <typename UNIT_TYPE, typename SCALE_TYPE>
UNIT_TYPE rescale(UNIT_TYPE value, SCALE_TYPE old_scale, SCALE_TYPE new_scale);

template <typename UNIT_TYPE, typename SCALE_TYPE, typename SUFFIX_TYPE>
class measurement {
protected:
    /** The protected internal value */
    UNIT_TYPE _value;

public:
    /** A read-only reference of the value at the default scale */
    const UNIT_TYPE& value;

protected:
    /** The constant string which has our unit */
    const char* const _suffix;

public:
    /** The declared type of the value */
    using type = UNIT_TYPE;

    /** Default Constructor */
    measurement() : _value{0}, value{_value}, _suffix{SUFFIX_TYPE::suffix} {
    }
    /** Valued Constructor */
    measurement(UNIT_TYPE v) : _value{v}, value{_value}, _suffix{SUFFIX_TYPE::suffix} {
    }
    /** Const Copy Constructor */
    measurement(const measurement& other) : _value{other.value}, value{_value}, _suffix{other._suffix} {
    }
    /** Copy Constructor */
    measurement(measurement& other) : _value{other.value}, value{_value}, _suffix{other._suffix} {
    }
    /** Move Constructor */
    measurement(measurement&& other) : _value{other.value}, value{_value}, _suffix{std::move(other._suffix)} {
    }
    /** Destructor */
    virtual ~measurement() {
    }

    // Don't allow one measurement to be transferred to another by default,
    // Child classes must overload any operators

    /** Returns the units as a string */
    const char* get_suffix() const {
        return _suffix;
    }

    /** Returns the value at a new scale */
    UNIT_TYPE at_scale(SCALE_TYPE new_scale) const {
        return rescale(_value, SCALE_TYPE::none, new_scale);
    }

    /** Returns the value at the default scale */
    UNIT_TYPE operator()() const {
        return _value;
    }

    /** Returns the value at the default scale */
    virtual UNIT_TYPE reduce() const {
        return _value;
    }
};

/** Use the streams namespace to import the stream output operator */
namespace streams {
/** This allow the measurement object to be able to be printed via streams */
template <typename UNIT_TYPE, typename SCALE_TYPE, typename SUFFIX_TYPE>
std::ostream& operator<<(std::ostream& output, const iso::measurement<UNIT_TYPE, SCALE_TYPE, SUFFIX_TYPE>& obj) {
    return output << obj.value << "_" << obj.get_suffix();
}

}  // namespace streams

}  // namespace iso
