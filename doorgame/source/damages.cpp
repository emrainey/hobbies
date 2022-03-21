#include "doorgame/damages.hpp"

namespace doorgame {

Damages valid_damages = {Damage::None,       Damage::Sting,    Damage::Bite,  Damage::Cut,
                         Damage::Laceration, Damage::Moderate, Damage::Heavy, Damage::Fatal};

std::ostream& operator<<(std::ostream& os, Damage value) {
    switch (value) {
        case Damage::None:
            os << "None";
            break;
        case Damage::Sting:
            os << "Sting";
            break;
        case Damage::Bite:
            os << "Bite";
            break;
        case Damage::Cut:
            os << "Cut";
            break;
        case Damage::Laceration:
            os << "Laceration";
            break;
        case Damage::Moderate:
            os << "Moderate";
            break;
        case Damage::Heavy:
            os << "Heavy";
            break;
        case Damage::Fatal:
            os << "Fatal";
            break;
    }
    os << " (" << basal::to_underlying(value) << ") ";
    return os;
}

void print(Damages damages = valid_damages) {
    std::cout << "Damage:" << std::endl;
    for (auto d : damages) {
        std::cout << "\t" << d << std::endl;
    }
}

bool is_valid(Damage damage) {
    for (auto d : valid_damages) {
        if (d == damage) {
            return true;
        }
    }
    return false;
}

Damage choose(Damages damages = valid_damages) {
    print(damages);
    char c;
    std::cin >> c;
    Damage d = static_cast<Damage>(c);
    // FIXME round up or down to nearest enum?
    return (is_valid(d) ? d : Damage::None);
}

}  // namespace doorgame