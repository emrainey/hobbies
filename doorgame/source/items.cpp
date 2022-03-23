#include "doorgame/items.hpp"

namespace doorgame {

Items valid_items = {Item::Nothing, Item::Torch, Item::Key, Item::Book, Item::Sword, Item::Potion, Item::Bomb};

bool is_valid(Item item) {
    for (auto a : valid_items) {
        if (a == item) {
            return true;
        }
    }
    return false;
}

}  // namespace doorgame
