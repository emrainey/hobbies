#include "doorgame/health.hpp"

#include <basal/basal.hpp>

namespace doorgame {

std::ostream& operator<<(std::ostream& os, Health value) {
    size_t h = basal::to_underlying(value);
    if (value == Health::Dead) {
        os << "Dead";
    } else if (basal::to_underlying(Health::Dead) < h and h <= basal::to_underlying(Health::Critical)) {
        os << "Critical";
    } else if (basal::to_underlying(Health::Critical) < h and h <= basal::to_underlying(Health::Injured)) {
        os << "Injured";
    } else if (basal::to_underlying(Health::Injured) < h and h <= basal::to_underlying(Health::Hurt)) {
        os << "Hurt";
    } else if (basal::to_underlying(Health::Hurt) < h and h <= basal::to_underlying(Health::Bruised)) {
        os << "Bruised";
    } else if (basal::to_underlying(Health::Bruised) < h and h <= basal::to_underlying(Health::Normal)) {
        os << "Normal";
    }
    return os;
}

bool is_valid(Health amount) {
    return (basal::to_underlying(Health::Dead) <= basal::to_underlying(amount) and
            basal::to_underlying(amount) <= basal::to_underlying(Health::Normal));
}

}  // namespace doorgame
