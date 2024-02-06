/// @file
/// Enumeration definition and helpers
/// @copyright Copyright (c) 2022

#pragma once

#include <iostream>
#include <vector>

namespace doorgame {

enum class Health : size_t
{
    Dead = 0,
    Critical = 10,
    Injured = 20,
    Hurt = 30,
    Bruised = 40,
    Normal = 50
};

std::ostream& operator<<(std::ostream& os, Health value);
bool is_valid(Health amount);

}  // namespace doorgame
