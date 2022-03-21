#include "doorgame/targets.hpp"

#include <basal/basal.hpp>

namespace doorgame {

Targets valid_targets = {Target::None, Target::Self, Target::Room, Target::Monster, Target::Item};

std::ostream& operator<<(std::ostream& os, Target target) {
    switch (target) {
        case Target::None:
            os << "None";
            break;
        case Target::Self:
            os << "Self";
            break;
        case Target::Room:
            os << "Room";
            break;
        case Target::Monster:
            os << "Monster";
            break;
        case Target::Item:
            os << "Item";
            break;
        default:
            os << "Unknown target type (" << basal::to_underlying(target) << ") ";
    }
    return os;
}

void print_targets(Targets targets) {
    std::cout << "Targets: " << std::endl;
    for (auto& target : targets) {
        std::cout << "\t" << target << std::endl;
    }
}

bool is_valid(Target target) {
    for (auto t : valid_targets) {
        if (t == target) {
            return true;
        }
    }
    return false;
}

Target choose(Targets targets) {
    print_targets(targets);
    char c;
    std::cin >> c;
    Target t = static_cast<Target>(c);
    return (is_valid(t) ? t : Target::None);
}

}  // namespace doorgame