
#include <htm/htm.hpp>

// helper
static std::tm const* now() {
    std::time_t t = std::time(nullptr);
    return std::localtime(&t);
}

namespace htm {

namespace encoders {
// TODO fix for leapyears, see leap year rules 4th, 100th, 400th, etc
sparse_word<365> dayofyear() {
    std::tm const* const tm = now();
    uint32_t yday = static_cast<uint32_t>(tm->tm_yday);
    return scalar<365>(yday, 7);  // 2% "on"
}

sparse_word<31> dayofmonth() {
    std::tm const* const tm = now();
    uint32_t mday = static_cast<uint32_t>(tm->tm_mday);
    return scalar<31>(mday, 3);  // ~10% "on"
}

sparse_word<7> dayofweek() {
    std::tm const* const tm = now();
    uint32_t wday = static_cast<uint32_t>(tm->tm_wday);
    return scalar<7>(wday, 1);  // 1/7 "on"
}

sparse_word<365 + 31 + 7> dayof() {
    std::tm const* const tm = now();
    auto u1 = scalar<365>(static_cast<uint32_t>(tm->tm_yday), 7);
    auto u2 = scalar<31>(static_cast<uint32_t>(tm->tm_mday), 3);
    auto u3 = scalar<7>(static_cast<uint32_t>(tm->tm_wday), 1);
    return (u1 + u2) + u3;
}
}  // namespace encoders
}  // namespace htm
