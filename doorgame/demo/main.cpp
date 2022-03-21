#include "doorgame/doorgame.hpp"

using namespace doorgame;

// load the doors and the items
// +-+-+-+
// |0;1:2|
// +-+:+:+
// |3:4:5|
// +:+:+:+
// |6;7|8|
// +-+-+-+
Doors doors = {
    std::make_tuple(0, Direction::East, 1),
    std::make_tuple(1, Direction::East, 2, Direction::West),
    std::make_tuple(1, Direction::South, 4, Direction::North),
    std::make_tuple(2, Direction::South, 5, Direction::North),
    std::make_tuple(3, Direction::East, 4, Direction::West),
    std::make_tuple(3, Direction::South, 6, Direction::North),
    std::make_tuple(4, Direction::South, 7, Direction::North),
    std::make_tuple(4, Direction::East, 5, Direction::West),
    std::make_tuple(5, Direction::South, 8, Direction::North),
    std::make_tuple(7, Direction::West, 6),
};
Stuff stuff = {
    std::make_tuple(Item::Torch, 0),
    std::make_tuple(Item::Book, 1),
    std::make_tuple(Item::Key, 4),
};

MonsterList monsterlist = {6};

int main(int argc __attribute__((unused)), char* argv[] __attribute__((unused))) {
    // TODO load some game from a file.
    Game game(0, 8, 9, doors, stuff, monsterlist);
    game.execute();
    return 0;
}
