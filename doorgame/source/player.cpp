#include "doorgame/player.hpp"

namespace doorgame {

Player::Player(size_t loc) : Storage(3), Animate(loc) {
}  // starts empty

std::ostream& Player::stream(std::ostream& os) const {
    return Animate::stream(Storage::stream(os));
}

}  // namespace doorgame