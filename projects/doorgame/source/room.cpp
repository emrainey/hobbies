#include "doorgame/room.hpp"

#include <basal/basal.hpp>

namespace doorgame {

constexpr static bool load_debug = false;

Room::Room(size_t id) : Storage{1}, m_id{id}, doors{id, id, id, id}, has_investigated{false} {
}  // starts empty

void Room::set_adjacent(size_t other_id, Direction d) {
    if constexpr (load_debug) {
        fprintf(stderr, "[DEBUG] Connecting room %zu (%p) to room %zu on the %c side\r\n", m_id, (void *)this, other_id,
                basal::to_underlying(d));
    }
    if (d == Direction::North) {
        doors[0] = other_id;
    } else if (d == Direction::South) {
        doors[1] = other_id;
    } else if (d == Direction::East) {
        doors[2] = other_id;
    } else if (d == Direction::West) {
        doors[3] = other_id;
    }
}

size_t Room::get_adjacent(Direction d) {
    if (d == Direction::North) {
        return doors[0];
    } else if (d == Direction::South) {
        return doors[1];
    } else if (d == Direction::East) {
        return doors[2];
    } else if (d == Direction::West) {
        return doors[3];
    } else {  // here or anything else
        return m_id;
    }
}

size_t Room::get_id() const {
    return m_id;
}

void Room::investigated() {
    has_investigated = true;
}

bool Room::is_investigated() const {
    return has_investigated;
}

Directions Room::get_directions() const {
    Directions directions;
    directions.push_back(Direction::Here);
    if (doors[0] != m_id) {
        directions.push_back(Direction::North);
    }
    if (doors[1] != m_id) {
        directions.push_back(Direction::South);
    }
    if (doors[2] != m_id) {
        directions.push_back(Direction::East);
    }
    if (doors[3] != m_id) {
        directions.push_back(Direction::West);
    }
    return directions;
}

}  // namespace doorgame