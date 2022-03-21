/**
 * @file
 * Game Header
 * @copyright Copyright (c) 2022
 */

#pragma once

#include <iostream>
#include <vector>

#include "doorgame/actions.hpp"
#include "doorgame/map.hpp"
#include "doorgame/monster.hpp"
#include "doorgame/parameters.hpp"
#include "doorgame/player.hpp"

namespace doorgame {

class Game {
public:
    Game(size_t start, size_t end, size_t num_rooms, Doors doorlist, Stuff stuff, MonsterList monsterlist);
    void process(std::pair<Action, Parameter> event);
    std::pair<Action, Parameter> ask_action();
    Targets get_targets();
    void execute(void);
    std::ostream& stream(std::ostream& os) const;

protected:
    size_t start;
    Player player;
    Monsters monsters;
    Map map;
    bool is_playing;
};

}  // namespace doorgame
