/// @file
/// Enumeration definition and helpers
/// @copyright Copyright (c) 2022

#pragma once

#include <iostream>
#include <vector>

namespace doorgame {

enum class Item : char {
    Nothing = 'n',
    Torch = 't',
    Key = 'k',
    Book = 'b',
    Sword = 's',
    Potion = 'p',
    Bomb = 'm'
};

using Items = std::vector<Item>;

extern Items valid_items;

bool is_valid(Item item);

}  // namespace doorgame
