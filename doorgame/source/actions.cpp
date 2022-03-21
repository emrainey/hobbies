#include "doorgame/actions.hpp"

namespace doorgame {

Actions valid_actions = {
    Action::Nothing, Action::Move, Action::Investigate, Action::Pickup, Action::Attack, Action::Use, Action::Quit,
};

std::ostream& operator<<(std::ostream& os, Action a) {
    switch (a) {
        case Action::Nothing:
            os << "[n]othing";
            break;
        case Action::Move:
            os << "[m]ove";
            break;
        case Action::Investigate:
            os << "[i]nvestigate";
            break;
        case Action::Pickup:
            os << "[p]ickup";
            break;
        case Action::Attack:
            os << "[a]ttack";
            break;
        case Action::Use:
            os << "[u]se";
            break;
        case Action::Quit:
            os << "[q]uit";
            break;
        default:
            os << "Unknown Action " << (char)a;
            break;
    }
    return os;
}

void print(Actions actions) {
    std::cout << "Actions: ";
    for (auto a : actions) {
        std::cout << a << " ";
    }
}

bool is_valid(Action action) {
    for (auto a : valid_actions) {
        if (a == action) {
            return true;
        }
    }
    return false;
}

Action choose(Actions actions) {
    print(actions);
    std::cout << " ? " << std::flush;
    char c;
    std::cin >> c;
    Action a = static_cast<Action>(c);
    return (is_valid(a) ? a : Action::Nothing);
}

}  // namespace doorgame
