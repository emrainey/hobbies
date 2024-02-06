///
/// @file
/// Storage Header
/// @copyright Copyright (c) 2022

///
#pragma once

#include <cstddef>

#include "doorgame/items.hpp"

namespace doorgame {

using Inventory = std::vector<Item>;  // FIXME should be a bit more complex

class Storage {
public:
    Storage(size_t limit);
    virtual ~Storage() = default;
    const Inventory& get_inventory() const;
    bool add(Item item);
    bool remove(Item item);
    const Storage& as_storage() const;
    Storage& as_storage();

protected:
    Inventory inventory;
};

}  // namespace doorgame