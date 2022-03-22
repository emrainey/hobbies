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

void Game::process(std::pair<Action, Parameter> event) {
    Action action = std::get<0>(event);
    Parameter param = std::get<1>(event);
    bool succeeded = false;
    view.attempt(action, param);
    switch (action) {
        case Action::Move: {
            // throw_exception_unless(std::holds_alternative<Direction>(param), "Must have a Direction");
            Direction dir = std::get<1>(param);
            succeeded = map.move(player, dir);
            break;
        }
        case Action::Pickup: {
            // throw_exception_unless(std::holds_alternative<Item>(param), "Must have an Item");
            Item item = std::get<2>(param);
            succeeded = player.add(item);
            break;
        }
        case Action::Investigate: {
            // throw_exception_unless(std::holds_alternative<Direction>(param), "Must have a Direction");
            Direction dir = std::get<1>(param);
            // the player investigates (looks) in a certain direction
            size_t place;
            if (map.get_adjacent(player, dir, place)) {
                map[place].investigated();
                succeeded = true;
            }
            break;
        }
        case Action::Attack: {
            // throw_exception_unless(std::holds_alternative<Target>(param), "Must have a Target");
            Target target = std::get<3>(param);
            if (target == Target::Self) {
                player.take(Damage::Sting);
            } else if (target == Target::Room) {
                // nothing happens yet
            } else if (target == Target::Monster) {
                // is there a monster in this room?
            }
            break;
        }
        case Action::Use: {
            // throw_exception_unless(std::holds_alternative<Item>(param), "Must have an Item");
            Item item = std::get<2>(param);
            if (item == Item::Torch) {
                map.get_room(player).investigated();
            } else if (item == Item::Book) {
                // nothing happens in game
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
    view.complete(action, param, succeeded);
}

std::pair<Action, Parameter> Game::ask_action() {
    Action action = view.choose(valid_actions);
    Parameter param;  // starts empty
    switch (action) {
        case Action::Move:
            param = view.choose(map.get_room(player).get_directions());
            break;
        case Action::Investigate:
            param = view.choose(map.get_room(player).get_directions());
            break;
        case Action::Pickup:
            param = view.choose(map.get_room(player).get_inventory());
            break;
        case Action::Attack:
            param = view.choose(get_targets());
            break;
        case Action::Use:
            param = view.choose(player.get_inventory());
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
        view.display(player);
        view.display(map.get_room(player));
        auto event = ask_action();
        process(event);
        if (map.is_done(player)) {
            is_playing = false;
        }
    }
}

}  // namespace doorgame
