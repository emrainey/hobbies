#include "doorgame/directions.hpp"

namespace doorgame {

Directions valid_directions = {
    Direction::Here, Direction::North, Direction::South, Direction::East, Direction::West,
};

std::ostream& operator<<(std::ostream& os, Direction d) {
    switch (d) {
        case Direction::Here:
            os << "[h]ere";
            break;
        case Direction::North:
            os << "[n]orth";
            break;
        case Direction::South:
            os << "[s]outh";
            break;
        case Direction::East:
            os << "[e]ast";
            break;
        case Direction::West:
            os << "[w]est";
            break;
        default:
            os << "Unknown Direction " << (char)d;
            break;
    }
    return os;
}

void print(Directions directions) {
    std::cout << "Direction:" << std::endl;
    for (auto d : directions) {
        std::cout << "\t" << d << std::endl;
    }
}

bool is_valid(Direction direction) {
    for (auto d : valid_directions) {
        if (d == direction) {
            return true;
        }
    }
    return false;
}

Direction choose(Directions directions) {
    print(directions);
    char c;
    std::cin >> c;
    Direction d = static_cast<Direction>(c);
    return (is_valid(d) ? d : Direction::Here);
}

}  // namespace doorgame