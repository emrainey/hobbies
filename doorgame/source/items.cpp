#include "doorgame/items.hpp"

namespace doorgame {

Items valid_items = {Item::Nothing, Item::Torch, Item::Key, Item::Book};

std::ostream& operator<<(std::ostream& os, Item i) {
    switch (i) {
        case Item::Nothing:
            os << "[n]othing";
            break;
        case Item::Torch:
            os << "[t]orch";
            break;
        case Item::Key:
            os << "[k]ey";
            break;
        case Item::Book:
            os << "[b]ook";
            break;
        default:
            os << "Unknown item (" << (int)i << ")";
            break;
    }
    return os;
}

void print(Items items) {
    std::cout << "Items: " << std::endl;
    for (auto i : items) {
        std::cout << "\t" << i << std::endl;
    }
}

bool is_valid(Item item) {
    for (auto a : valid_items) {
        if (a == item) {
            return true;
        }
    }
    return false;
}

Item choose(Items items) {
    print(items);
    char c;
    std::cin >> c;
    Item i = static_cast<Item>(c);
    return (is_valid(i) ? i : Item::Nothing);
}

}  // namespace doorgame
