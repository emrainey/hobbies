
#include "iso/iso.hpp"

#include "iso/compound.hpp"
namespace iso {

template <>
double rescale(double value, SI::prefix old_scale, SI::prefix new_scale) {
    double diff = static_cast<typename std::underlying_type<SI::prefix>::type>(old_scale)
                  - static_cast<typename std::underlying_type<SI::prefix>::type>(new_scale);
    return value * pow(10.0, diff);
}

template <>
uint64_t rescale(uint64_t value, IEC::prefix old_scale, IEC::prefix new_scale) {
    int diff = static_cast<typename std::underlying_type<SI::prefix>::type>(old_scale)
               - static_cast<typename std::underlying_type<SI::prefix>::type>(new_scale);
    if (diff > 0) {
        return value << uint32_t(diff);
    } else if (diff < 0) {
        return value >> uint32_t(std::abs(diff));
    } else {
        return value;
    }
}

namespace literals {
// Compound Quote operator
speed operator""_m_per_sec(long double a) {
    return speed{static_cast<double>(a), 1.0};
}

/// Define shortcut for Gravity values!
acceleration operator""_G(long double g) {
    using namespace iso::operators;
    return (distance(g) / 1.0_sec) / 1.0_sec;
}
}  // namespace literals

// Compound Constructors
namespace operators {

speed operator/(const distance &num, const time &denom) {
    return speed{num, denom};
}

speed operator/(distance &&num, time &&denom) {
    return speed{num, denom};
}

speed operator/(const distance &num, time &&denom) {
    return speed{num, denom};
}

speed operator/(distance &&num, const time &denom) {
    return speed{num, denom};
}

// const copy of an alternative speed
rate<feet, seconds> operator/(const feet &num, const seconds &denom) {
    return rate<feet, seconds>(num, denom);
}

acceleration operator/(speed &&num, time &&denom) {
    return acceleration{num, denom};
}

acceleration operator/(const speed &num, const time &denom) {
    return acceleration{num, denom};
}

acceleration operator/(const speed &num, time &&denom) {
    return acceleration{num, denom};
}

acceleration operator/(speed &&num, const time &denom) {
    return acceleration{num, denom};
}

torque operator*(const newtons &N, const meters &m) {
    return torque{N, m};
}

torque operator*(newtons &&N, meters &&m) {
    return torque{N, m};
}

torque operator*(const newtons &N, meters &&m) {
    return torque{N, m};
}

torque operator*(newtons &&N, const meters &m) {
    return torque{N, m};
}

// This is to override specific relations where things ARE equal
bool operator==(const joules &J, const torque &T) {
    return equivalent(J.value, T.reduce());
}

}  // namespace operators

}  // namespace iso
