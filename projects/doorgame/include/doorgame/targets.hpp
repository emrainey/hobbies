/// @file
/// Enumeration definition and helpers
/// @copyright Copyright (c) 2022

#pragma once

#include <iostream>
#include <vector>

namespace doorgame {

enum class Target : char
{
    None = 'n',
    Player = 'p',
    Room = 'r',
    Monster = 'm',
    Item = 'i'
};

using Targets = std::vector<Target>;

extern Targets valid_targets;

std::ostream& operator<<(std::ostream& os, Target target);
void print_targets(Targets targets = valid_targets);
bool is_valid(Target target);
Target choose(Targets targets = valid_targets);

}  // namespace doorgame
