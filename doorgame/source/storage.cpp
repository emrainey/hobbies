#include "doorgame/storage.hpp"

namespace doorgame {

Storage::Storage(size_t limit) : inventory(limit, Item::Nothing) {
}

const Inventory& Storage::get_inventory() const {
    return inventory;
}

bool Storage::add(Item item) {
    // replace the first non-nothing item
    auto slot = std::find(std::begin(inventory), std::end(inventory), Item::Nothing);
    if (slot != std::end(inventory)) {
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

const Storage& Storage::as_storage() const {
    return (*this);
}

Storage& Storage::as_storage() {
    return (*this);
}

}  // namespace doorgame
