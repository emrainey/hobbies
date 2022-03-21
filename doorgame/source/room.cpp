#include "doorgame/room.hpp"

namespace doorgame {

constexpr static bool load_debug = true;

Room::Room(size_t id) : Storage(1), m_id(id), doors{id, id, id, id}, has_investigated(false) {
}  // starts empty

void Room::set_adjacent(size_t other_id, Direction d) {
    if constexpr (load_debug) {
        std::cerr << "Connecting room " << m_id << " (" << this << ") to room " << other_id << " on the " << d
                  << " direction" << std::endl;
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

std::ostream& Room::stream(std::ostream& os) const {
    os << "The doors in room " << m_id << " are ";
    if (doors[0] != m_id) {
        os << Direction::North << " (to room " << doors[0] << ") ";
    }
    if (doors[1] != m_id) {
        os << Direction::South << " (to room " << doors[1] << ") ";
    }
    if (doors[2] != m_id) {
        os << Direction::East << " (to room " << doors[2] << ") ";
    }
    if (doors[3] != m_id) {
        os << Direction::West << " (to room " << doors[3] << ") ";
    }
    os << "." << std::endl;
    if (has_investigated) {
        return Storage::stream(os);
    } else {
        return os;
    }
}

size_t Room::get_id() const {
    return m_id;
}

void Room::investigated() {
    has_investigated = true;
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