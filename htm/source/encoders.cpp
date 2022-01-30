
#include <htm/htm.hpp>

namespace htm {
    const std::tm * const now() {
        std::time_t t = std::time(nullptr);
        return std::localtime(&t);
    }

    // TODO fix for leapyears, see leap year rules 4th, 100th, 400th, etc
    sparse_word<365> dayofyear_encoder() {
        const std::tm * const tm = now();
        uint32_t yday = tm->tm_yday;
        return scalar_encoder<365>(yday, 7); // 2% "on"
    }

    sparse_word<31> dayofmonth_encoder() {
        const std::tm * const tm = now();
        uint32_t mday = tm->tm_mday;
        return scalar_encoder<31>(mday, 3); // ~10% "on"
    }

    sparse_word<7> dayofweek_encoder() {
        const std::tm * const tm = now();
        uint32_t wday = tm->tm_wday;
        return scalar_encoder<7>(wday, 1); // 1/7 "on"
    }

    sparse_word<365+31+7> dayof_encoder() {
        const std::tm * const tm = now();
        auto u1 = scalar_encoder<365>(tm->tm_yday, 7);
        auto u2 = scalar_encoder<31>(tm->tm_mday, 3);
        auto u3 = scalar_encoder<7>(tm->tm_wday, 1);
        return (u1 << u2) << u3;
    }
}
