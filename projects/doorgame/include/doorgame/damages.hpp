/// @file
/// Enumeration definition and helpers
/// @copyright Copyright (c) 2022

#pragma once

#include <iostream>
#include <vector>

namespace doorgame {

/// The enumerated value of Damage that an Animate object can take
enum class Damage : size_t
{
    None = 0,
    Sting = 1,
    Bite = 5,
    Cut = 7,
    Laceration = 10,
    Moderate = 20,
    Heavy = 30,
    Fatal = 50,
};

/// A set of damage
using Damages = std::vector<Damage>;

/// The set of all possible damages
extern Damages valid_damages;

/// Is a damage contained in the possible set of damages
bool is_valid(Damage damage);

}  // namespace doorgame
