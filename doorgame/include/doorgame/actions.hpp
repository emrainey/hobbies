/**
 * @file
 * Enumeration definition and helpers
 * @copyright Copyright (c) 2022
 */

#pragma once

#include <iostream>
#include <vector>

namespace doorgame {

/** An enumeration of possible actions */
enum class Action : char
{
    Nothing = 'n',      //!< Do nothing
    Move = 'm',         //!< Move to another Room
    Investigate = 'i',  //!< Investigate a Room
    Pickup = 'p',       //!< Pickup an item from the Room
    Attack = 'a',       //!< Attack a Target
    Use = 'u',          //!< Use an item
    Quit = 'q'          //!< Quit the game
};

/** A set of actions */
using Actions = std::vector<Action>;

/** The complete set of all possible actions */
extern Actions valid_actions;

/** Is a given value of action within the valid set */
bool is_valid(Action action);

}  // namespace doorgame
