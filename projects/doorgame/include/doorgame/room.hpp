///
/// @file
/// Storage Header
/// @copyright Copyright (c) 2022

///
#pragma once

#include <cstddef>

#include "doorgame/directions.hpp"
#include "doorgame/storage.hpp"

namespace doorgame {

class Room : public Storage {
public:
    Room(size_t id);
    virtual ~Room() = default;
    void set_adjacent(size_t other_id, Direction d);
    size_t get_adjacent(Direction d);
    size_t get_id() const;
    void investigated();
    bool is_investigated() const;
    Directions get_directions() const;

protected:
    size_t m_id;
    size_t doors[4];
    bool has_investigated;
};

}  // namespace doorgame
