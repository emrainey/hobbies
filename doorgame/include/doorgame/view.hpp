#pragma once

#include "doorgame/actions.hpp"
#include "doorgame/animate.hpp"
#include "doorgame/damages.hpp"
#include "doorgame/directions.hpp"
#include "doorgame/items.hpp"
#include "doorgame/map.hpp"
#include "doorgame/monster.hpp"
#include "doorgame/parameters.hpp"
#include "doorgame/player.hpp"
#include "doorgame/room.hpp"
#include "doorgame/storage.hpp"
#include "doorgame/targets.hpp"

namespace doorgame {
/** The Input/Output Interface for the Game */
class View {
public:
    /** Called before an action completes (or is attempted) */
    virtual void attempt(Action action, Parameter param) noexcept(false) = 0;
    /** Called after an action completes (but may have failed) */
    virtual void complete(Action action, Parameter param, bool result) noexcept(false) = 0;

    /** @{ Display a set of enumerable things */
    virtual void display(const Actions& actions) = 0;
    virtual void display(const Damages& damages) = 0;
    virtual void display(const Directions& directions) = 0;
    virtual void display(const Items& items) = 0;
    virtual void display(const Targets& targets) = 0;
    /** @} */

    /** @{ Displays the state of an object */
    virtual void display(const Animate& object) = 0;
    // virtual void display(Game& game) = 0;
    virtual void display(const Room& room) = 0;
    virtual void display(const Storage& storage) = 0;
    virtual void display(const Map& map) = 0;
    virtual void display(const Player& player) = 0;
    virtual void display(const Monster& monster) = 0;
    /** @} */

    /** @{ Allows the user to choose a single item from a set of enumerable items */
    virtual Action choose(const Actions& actions) = 0;
    virtual Damage choose(const Damages& damages) = 0;
    virtual Direction choose(const Directions& directions) = 0;
    virtual Item choose(const Items& items) = 0;
    virtual Target choose(const Targets& targets) = 0;
    /** @} */
};

}  // namespace doorgame