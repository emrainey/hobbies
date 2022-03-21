/**
 * @file
 * A simple text adventure
 * @copyright 2022 Erik Rainey
 */

#include "doorgame/game.hpp"

namespace doorgame {

Game::Game() : start(0), player(start), monsters(), map(9, start, 8) {
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
    // put a monster in rooms
    monsters.emplace_back(6);
    map.load(doors, stuff);
}

void Game::process(std::pair<Action, Parameter> event) {
    Action action = std::get<0>(event);
    Parameter param = std::get<1>(event);
    switch (action) {
        case Action::Move: {
            Direction dir;
            if (std::holds_alternative<Direction>(param)) {
                dir = std::get<1>(param);
                if (map.move(player, dir)) {
                    // move the player from where they are to the new place
                    std::cout << "Player moved " << dir << std::endl;
                }
            }
            break;
        }
        case Action::Pickup: {
            Item item;
            if (std::holds_alternative<Item>(param)) {
                item = std::get<2>(param);
                std::cout << "Player got " << item << std::endl;
                player.add(item);
                player.stream(std::cout);
                std::cout << std::endl;
            }
        }
        case Action::Investigate: {
            Direction dir;
            if (std::holds_alternative<Direction>(param)) {
                dir = std::get<1>(param);
                // the player investigates (looks) in a certain direction
                std::cout << "Player investigated " << dir << std::endl;
                size_t place;
                if (map.get_adjacent(player, dir, place)) {
                    map[place].investigated();
                    map[place].stream(std::cout);
                } else {
                    std::cout << "No door in that direction.";
                }
                std::cout << std::endl;
            }
            break;
        }
        case Action::Attack:
            Target target;
            if (std::holds_alternative<Target>(param)) {
                target = std::get<3>(param);
                if (target == Target::Self) {
                    std::cout << "Player has hurt themselves in their confusion!" << std::endl;
                    player.take(Damage::Sting);
                } else if (target == Target::Room) {
                    std::cout << "Player flails about as if a spider was crawling on your neck" << std::endl;
                } else if (target == Target::Monster) {
                    // is there a monster in this room?
                }
            }
            break;
        case Action::Use:
            Item item;
            if (std::holds_alternative<Item>(param)) {
                item = std::get<2>(param);
                if (item == Item::Torch) {
                    std::cout << "Player has used lit torch and can see the room better" << std::endl;
                    map.get_room(player).investigated();
                } else if (item == Item::Book) {
                    std::cout << "Player has read a book about the amazing world of tax derivatives during the "
                                 "middle-post modern era"
                              << std::endl;
                }
            }
            break;
        case Action::Quit:
            exit(0);
            break;
        case Action::Nothing:
            std::cout << "You stare at the walls in delight." << std::endl;
        default:
            break;
    }
}

std::pair<Action, Parameter> Game::ask_action() {
    Action action = choose(valid_actions);
    Parameter param;  // starts empty
    switch (action) {
        case Action::Move:
            param = choose(map.get_room(player).get_directions());
            break;
        case Action::Investigate:
            param = choose(map.get_room(player).get_directions());
            break;
        case Action::Pickup:
            param = choose(map.get_room(player).get_inventory());
            break;
        case Action::Attack:
            param = choose(get_targets());
            break;
        case Action::Use:
            param = choose(player.get_inventory());
            break;
        default:
            // nothing extra to ask
            break;
    }
    return std::make_pair(action, param);
}

Targets Game::get_targets() {
    Targets targets;  // start empty
    targets.push_back(Target::None);
    targets.push_back(Target::Self);
    Room& room = map.get_room(player);
    if (room.get_inventory().size() > 0) {
        targets.push_back(Target::Item);
    }
    for (auto& monster : monsters) {
        if (monster.location() == player.location()) {
            targets.push_back(Target::Monster);
        }
    }
    return targets;
}

void Game::execute(void) {
    while (is_playing) {
        stream(std::cout);
        auto event = ask_action();
        std::cout << "Chose " << std::get<0>(event) << " and " << std::get<1>(event) << std::endl;
        process(event);
        if (map.is_done(player)) {
            is_playing = false;
        }
    }
}

std::ostream& Game::stream(std::ostream& os) const {
    os << "Game state: " << std::endl;
    os << "Player: ";
    player.stream(os);
    os << std::endl;
    map.get_room(player).stream(os);
    os << std::endl;
    return os;
}

}  // namespace doorgame
