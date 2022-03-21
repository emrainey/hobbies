/**
 * @file
 * Enumeration definition and helpers
 * @copyright Copyright (c) 2022
 */

#pragma once

#include <iostream>
#include <vector>

namespace doorgame {

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

using Damages = std::vector<Damage>;

extern Damages valid_damages;

std::ostream& operator<<(std::ostream& os, Damage value);
void print(Damages damages = valid_damages);
bool is_valid(Damage damage);
Damage choose(Damages damages = valid_damages);

}  // namespace doorgame
