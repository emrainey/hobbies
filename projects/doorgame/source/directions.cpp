#include "doorgame/directions.hpp"

namespace doorgame {

Directions valid_directions = {
    Direction::Here, Direction::North, Direction::South, Direction::East, Direction::West,
};

bool is_valid(Direction direction) {
    for (auto d : valid_directions) {
        if (d == direction) {
            return true;
        }
    }
    return false;
}

}  // namespace doorgame