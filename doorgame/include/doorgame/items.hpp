/**
 * @file
 * Enumeration definition and helpers
 * @copyright Copyright (c) 2022
 */

#pragma once

#include <iostream>
#include <vector>

namespace doorgame {

enum class Item : char
{
    Nothing = 'n',
    Torch = 't',
    Key = 'k',
    Book = 'b'
};

using Items = std::vector<Item>;

extern Items valid_items;

std::ostream& operator<<(std::ostream& os, Item i);
void print(Items items = valid_items);
bool is_valid(Item item);
Item choose(Items items = valid_items);

}  // namespace doorgame
