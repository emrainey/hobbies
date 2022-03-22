/**
 * @file
 * Enumeration definition and helpers
 * @copyright Copyright (c) 2022
 */

#pragma once

#include <iostream>
#include <vector>

namespace doorgame {

enum class Direction : char
{
    Here = 'h',
    North = 'n',
    South = 's',
    East = 'e',
    West = 'w'
};

using Directions = std::vector<Direction>;

extern Directions valid_directions;

bool is_valid(Direction direction);

}  // namespace doorgame
