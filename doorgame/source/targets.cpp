#include "doorgame/targets.hpp"

#include <basal/basal.hpp>

namespace doorgame {

Targets valid_targets = {Target::None, Target::Player, Target::Room, Target::Monster, Target::Item};

bool is_valid(Target target) {
    for (auto t : valid_targets) {
        if (t == target) {
            return true;
        }
    }
    return false;
}

}  // namespace doorgame