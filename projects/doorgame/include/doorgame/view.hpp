#pragma once

#include "doorgame/animate.hpp"
#include "doorgame/events.hpp"
#include "doorgame/map.hpp"
#include "doorgame/monster.hpp"
#include "doorgame/player.hpp"
#include "doorgame/room.hpp"
#include "doorgame/storage.hpp"

namespace doorgame {
/// The Input/Output Interface for the Game
class View {
public:
    /// Called before an action completes (or is attempted)
    virtual void attempt(Event event) noexcept(false) = 0;
    /// Called after an action completes (but may have failed)
    virtual void complete(Event event, bool result) noexcept(false) = 0;

    /// @{ Display a set of enumerable things
    virtual void display(Actions const& actions) = 0;
    virtual void display(Damages const& damages) = 0;
    virtual void display(Directions const& directions) = 0;
    virtual void display(Items const& items) = 0;
    virtual void display(Targets const& targets) = 0;
    /// @}

    /// @{ Displays the state of an object
    virtual void display(Animate const& object, std::string preface) = 0;
    // virtual void display(Game const& game) = 0;
    virtual void display(Room const& room) = 0;
    virtual void display(Storage const& storage, std::string preface = "") = 0;
    virtual void display(Map const& map) = 0;
    virtual void display(Player const& player) = 0;
    virtual void display(Monster const& monster) = 0;
    /// @}

    /// @{ Allows the user to choose a single item from a set of enumerable items
    virtual Action choose(Actions const& actions) = 0;
    virtual Damage choose(Damages const& damages) = 0;
    virtual Direction choose(Directions const& directions) = 0;
    virtual Item choose(Items const& items) = 0;
    virtual Target choose(Targets const& targets) = 0;
    /// @}

    virtual void greeting(void) = 0;
    virtual void completed(void) = 0;
    virtual void goodbye(void) = 0;
};

}  // namespace doorgame