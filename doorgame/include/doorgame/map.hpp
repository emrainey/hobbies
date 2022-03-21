/**
 * @file
 * map header
 * @copyright Copyright (c) 2022
 */

#pragma once

#include <tuple>
#include <variant>
#include <vector>

#include "doorgame/animate.hpp"
#include "doorgame/room.hpp"

namespace doorgame {

// Start_Room_Index, Direction, End_Room_Index
using OneWayDoor = std::tuple<size_t, Direction, size_t>;

// Start_Room_Index, Direction (forward), End_Room_Index, Direction (back)
using TwoWayDoor = std::tuple<size_t, Direction, size_t, Direction>;

// Either one way doors or two way doors
using Door = std::variant<OneWayDoor, TwoWayDoor>;

// A list of doors
using Doors = std::vector<Door>;

// An item is in a particular room
using ItemLocation = std::tuple<Item, size_t>;

// The list of items and where they should go
using Stuff = std::vector<ItemLocation>;

class Map {
public:
    Map(size_t num, size_t start, size_t end);
    Room& operator[](size_t index) noexcept(false);
    Room& get_room(Animate& object);
    const Room& get_room(const Animate& object) const;
    bool get_adjacent(Animate& object, Direction dir, size_t& index);
    bool is_done(Animate& object) const;
    bool move(Animate& object, Direction dir);
    bool load(Doors doors, Stuff stuff) noexcept(false);
    std::ostream& stream(std::ostream& os) const;

protected:
    std::vector<Room> m_rooms;
    size_t m_start;
    size_t m_end;
};

}  // namespace doorgame