/**
 * @file
 * A simple text adventure
 * @copyright 2022 Erik Rainey
 */

#include "doorgame/game.hpp"

#include <basal/basal.hpp>
namespace doorgame {

Game::Game(View& view, size_t start, size_t end, size_t num_rooms, Doors doors, Stuff stuff, MonsterList monsterlist)
    : view(view), start(start), player(start), monsters(), map(num_rooms, start, end), is_playing(true) {
    for (auto& ml : monsterlist) {
        monsters.push_back(ml);
    }
    map.load(doors, stuff);
}

void Game::process(Event event) {
    Target subject = std::get<0>(event);
    Action action = std::get<1>(event);
    Target object = std::get<2>(event);
    Parameter param = std::get<3>(event);

    bool succeeded = false;
    view.attempt(event);
    switch (subject) {
        case Target::Player: {
            switch (action) {
                case Action::Move: {
                    if (object == Target::Player) {
                        // throw_exception_unless(std::holds_alternative<Direction>(param), "Must have a Direction");
                        Direction dir = std::get<1>(param);
                        if (dir == Direction::Here) {
                            succeeded = true;
                        } else {
                            succeeded = map.move(player, dir);
                        }
                    }
                    break;
                }
                case Action::Pickup: {
                    if (object == Target::Item) {
                        // throw_exception_unless(std::holds_alternative<Item>(param), "Must have an Item");
                        Item item = std::get<2>(param);
                        succeeded = player.add(item);
                    }
                    break;
                }
                case Action::Look: {
                    // throw_exception_unless(std::holds_alternative<Direction>(param), "Must have a Direction");
                    // the player investigates (looks) in a certain direction
                    if (object == Target::Room) {
                        Direction dir = std::get<1>(param);
                        if (dir == Direction::Here) {
                            map.get_room(player).investigated();
                            view.display(map.get_room(player));
                            succeeded = true;
                        } else {
                            size_t place;
                            if (map.get_adjacent(player, dir, place)) {
                                map[place].investigated();
                                view.display(map[place]);
                                succeeded = true;
                            }
                        }
                    }
                    if (object == Target::Player) {
                        view.display(player);
                    }
                    break;
                }
                case Action::Attack: {
                    // throw_exception_unless(std::holds_alternative<Target>(param), "Must have a Target");
                    if (object == Target::Player) {
                        Damage dmg = std::get<3>(param);
                        player.take(dmg);
                    } else if (object == Target::Item) {
                        // break an object
                    } else if (object == Target::Room) {
                        Direction dir = std::get<1>(param);
                        // break a door down?
                    } else if (object == Target::Monster) {
                        // FIXME is there a monster in this room?
                    }
                    break;
                }
                case Action::Use: {
                    if (object == Target::Item) {
                        // throw_exception_unless(std::holds_alternative<Item>(param), "Must have an Item");
                        Item item = std::get<2>(param);
                        if (item == Item::Torch) {
                            map.get_room(player).investigated();
                        } else if (item == Item::Book) {
                            // nothing happens in game
                        }
                    }
                    break;
                }
                case Action::Quit:
                    is_playing = false;
                    break;
                case Action::Nothing:
                    succeeded = true;
                    [[fallthrough]];
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
    view.complete(event, succeeded);
}

Event Game::ask_event() {
    Action action = view.choose(valid_actions);
    Target object = Target::None;
    Parameter param;  // starts empty
    switch (action) {
        case Action::Move:
            object = Target::Player;
            param = view.choose(map.get_room(player).get_directions());
            break;
        case Action::Look:
            object = view.choose(get_targets());
            if (object == Target::Room) {
                if (map.get_room(player).is_investigated()) {
                    param = view.choose(map.get_room(player).get_directions());
                } else {
                    param = Direction::Here;
                }
            } else if (object == Target::Item) {
                // look at your own inventory?
                param = view.choose(player.get_inventory());
            }
            break;
        case Action::Pickup:
            object = Target::Item;
            param = view.choose(map.get_room(player).get_inventory());
            break;
        case Action::Attack:
            object = view.choose(get_targets());
            param = view.choose(valid_damages);
            break;
        case Action::Use:
            object = Target::Item;
            param = view.choose(player.get_inventory());
            break;
        default:
            // nothing extra to ask
            break;
    }
    return std::make_tuple(Target::Player, action, object, param);
}

Targets Game::get_targets() {
    Targets targets;  // start empty
    targets.push_back(Target::None);
    targets.push_back(Target::Player);
    targets.push_back(Target::Room);
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
        auto event = ask_event();
        process(event);
        if (map.is_done(player)) {
            is_playing = false;
        }
    }
}

}  // namespace doorgame
