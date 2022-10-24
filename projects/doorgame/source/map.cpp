#include "doorgame/map.hpp"

namespace doorgame {

Map::Map(size_t num, size_t start, size_t end) : m_rooms{}, m_start{start}, m_end{end} {
    for (size_t id = 0; id < num; id++) {
        m_rooms.emplace_back(id);
    }
}

Room& Map::operator[](size_t index) noexcept(false) {
    return m_rooms.at(index);
}

Room& Map::get_room(Animate& object) {
    return m_rooms.at(object.location());
}

const Room& Map::get_room(const Animate& object) const {
    return m_rooms.at(object.location());
}

bool Map::get_adjacent(Animate& object, Direction dir, size_t& index) {
    // shortcut determining if the direction is here
    if (dir == Direction::Here) {
        index = object.location();
        return true;
    }
    // now it should be one of the 4 directions.
    size_t tmp = get_room(object).get_adjacent(dir);
    if (tmp == object.location()) {
        // no door that way
        return false;
    }
    index = tmp;
    return true;
}

bool Map::is_done(Animate& object) const {
    return m_end == object.location();
}

bool Map::move(Animate& object, Direction dir) {
    // from the current move in the direction
    size_t there = get_room(object).get_adjacent(dir);
    if (object.location() == there) {
        return false;  // you're already here/there
    }
    object.location(there);
    return true;
}

bool Map::load(Doors doors, Stuff stuff) noexcept(false) {
    bool problem = false;
    // FUTURE find the maximum door index
    // allocate the right number of rooms

    // attach all the rooms to each other.
    for (auto& door : doors) {
        if (std::holds_alternative<OneWayDoor>(door)) {
            OneWayDoor way = std::get<0>(door);
            size_t start_index = std::get<0>(way);
            Direction start_dir = std::get<1>(way);
            size_t end_index = std::get<2>(way);
            if (start_index < m_rooms.size() and end_index < m_rooms.size()) {
                m_rooms.at(start_index).set_adjacent(end_index, start_dir);
            } else {
                problem = true;
            }
        } else if (std::holds_alternative<TwoWayDoor>(door)) {
            TwoWayDoor way = std::get<1>(door);
            size_t start_index = std::get<0>(way);
            Direction start_dir = std::get<1>(way);
            size_t end_index = std::get<2>(way);
            Direction end_dir = std::get<3>(way);
            if (start_index < m_rooms.size() and end_index < m_rooms.size()) {
                m_rooms.at(start_index).set_adjacent(end_index, start_dir);
                m_rooms.at(end_index).set_adjacent(start_index, end_dir);
            } else {
                problem = true;
            }
        }
    }
    for (auto& thing : stuff) {
        Item item = std::get<0>(thing);
        size_t place = std::get<1>(thing);
        if (place < m_rooms.size()) {
            if (not m_rooms.at(place).add(item)) {
                throw std::out_of_range("Inventory is full during load");
            }
        } else {
            problem = true;
        }
    }
    return not problem;
}

}  // namespace doorgame