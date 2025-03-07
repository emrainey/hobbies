
#include "iso/types.hpp"
#include "iso/iso.hpp"
#include "iso/compound.hpp"
namespace iso {

using namespace basal::literals;

template <>
precision rescale(precision value, SI::prefix old_scale, SI::prefix new_scale) {
    precision diff = static_cast<typename std::underlying_type<SI::prefix>::type>(old_scale)
                     - static_cast<typename std::underlying_type<SI::prefix>::type>(new_scale);
    return value * pow(10.0_p, diff);
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
    constexpr precision const C = 1.0_p;
    return speed{distance{static_cast<precision>(a)}, time{C}};
}

/// Define shortcut for Gravity values!
acceleration operator""_G(long double g) {
    using namespace iso::operators;
    return (distance(static_cast<precision>(g)) / 1.0_sec) / 1.0_sec;
}
}  // namespace literals

// Compound Constructors
namespace operators {

speed operator/(distance const& num, time const& denom) {
    return speed{num, denom};
}

speed operator/(distance&& num, time&& denom) {
    return speed{num, denom};
}

speed operator/(distance const& num, time&& denom) {
    return speed{num, denom};
}

speed operator/(distance&& num, time const& denom) {
    return speed{num, denom};
}

// const copy of an alternative speed
rate<feet, seconds> operator/(feet const& num, seconds const& denom) {
    return rate<feet, seconds>(num, denom);
}

acceleration operator/(speed&& num, time&& denom) {
    return acceleration{num, denom};
}

acceleration operator/(speed const& num, time const& denom) {
    return acceleration{num, denom};
}

acceleration operator/(speed const& num, time&& denom) {
    return acceleration{num, denom};
}

acceleration operator/(speed&& num, time const& denom) {
    return acceleration{num, denom};
}

torque operator*(newtons const& N, meters const& m) {
    return torque{N, m};
}

torque operator*(newtons&& N, meters&& m) {
    return torque{N, m};
}

torque operator*(newtons const& N, meters&& m) {
    return torque{N, m};
}

torque operator*(newtons&& N, meters const& m) {
    return torque{N, m};
}

// This is to override specific relations where things ARE equal
bool operator==(joules const& J, torque const& T) {
    return basal::equivalent(J.value, T.reduce());
}

}  // namespace operators

}  // namespace iso
