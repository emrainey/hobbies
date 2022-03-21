#include "doorgame/storage.hpp"

namespace doorgame {

constexpr static bool load_debug = true;

Storage::Storage(size_t limit) : inventory(limit, Item::Nothing) {
}

const Inventory& Storage::get_inventory() const {
    return inventory;
}

bool Storage::add(Item item) {
    // replace the first non-nothing item
    auto slot = std::find(std::begin(inventory), std::end(inventory), Item::Nothing);
    if (slot != std::end(inventory)) {
        if constexpr (load_debug) {
            std::cerr << "Adding " << item << " to inventory." << std::endl;
        }
        *slot = item;
        return true;
    }
    return false;
}

bool Storage::remove(Item item) {
    for (auto& inv : inventory) {
        if (inv == item) {
            inv = Item::Nothing;
            return true;
        }
    }
    return false;
}

std::ostream& Storage::stream(std::ostream& os) const {
    os << "Inventory: ";
    for (auto& inv : inventory) {
        os << inv << " ";
    }
    return os;
}
}  // namespace doorgame
