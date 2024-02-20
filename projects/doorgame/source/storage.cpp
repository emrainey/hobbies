#include "doorgame/storage.hpp"

namespace doorgame {

Storage::Storage(size_t limit) : inventory(limit, Item::Nothing) {
}

Inventory const& Storage::get_inventory() const {
    return inventory;
}

bool Storage::add(Item item) {
    // replace the first non-nothing item
    for (auto& inv : inventory) {
        if (inv == Item::Nothing) {
            inv = item;
            return true;
        }
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

Storage const& Storage::as_storage() const {
    return (*this);
}

Storage& Storage::as_storage() {
    return (*this);
}

}  // namespace doorgame
