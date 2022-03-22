#include "doorgame/health.hpp"

#include <basal/basal.hpp>

namespace doorgame {

bool is_valid(Health amount) {
    return (basal::to_underlying(Health::Dead) <= basal::to_underlying(amount) and
            basal::to_underlying(amount) <= basal::to_underlying(Health::Normal));
}

}  // namespace doorgame
