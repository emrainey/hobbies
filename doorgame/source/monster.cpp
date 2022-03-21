#include "doorgame/monster.hpp"

namespace doorgame {

Monster::Monster(size_t loc) : Storage(1), Animate(loc) {
}  // smaller inventory

std::ostream& Monster::stream(std::ostream& os) const {
    return Animate::stream(Storage::stream(os));
}

}  // namespace doorgame
