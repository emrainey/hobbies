#include "doorgame/damages.hpp"

namespace doorgame {

Damages valid_damages = {Damage::None,       Damage::Sting,    Damage::Bite,  Damage::Cut,
                         Damage::Laceration, Damage::Moderate, Damage::Heavy, Damage::Fatal};

bool is_valid(Damage damage) {
    for (auto d : valid_damages) {
        if (d == damage) {
            return true;
        }
    }
    return false;
}

}  // namespace doorgame