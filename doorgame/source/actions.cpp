#include "doorgame/actions.hpp"

namespace doorgame {

Actions valid_actions = {
    Action::Nothing, Action::Move, Action::Look, Action::Pickup, Action::Attack, Action::Use, Action::Quit,
};

bool is_valid(Action action) {
    for (auto a : valid_actions) {
        if (a == action) {
            return true;
        }
    }
    return false;
}

}  // namespace doorgame
