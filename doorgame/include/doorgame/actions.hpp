/**
 * @file
 * Enumeration definition and helpers
 * @copyright Copyright (c) 2022
 */

#pragma once

#include <iostream>
#include <vector>

namespace doorgame {

enum class Action : char
{
    Nothing = 'n',
    Move = 'm',
    Investigate = 'i',
    Pickup = 'p',
    Attack = 'a',
    Use = 'u',
    Quit = 'q'
};

using Actions = std::vector<Action>;

extern Actions valid_actions;

std::ostream& operator<<(std::ostream& os, Action d);
void print(Actions actions = valid_actions);
bool is_valid(Action action);
Action choose(Actions actions = valid_actions);

}  // namespace doorgame
