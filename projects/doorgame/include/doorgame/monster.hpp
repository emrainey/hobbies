#pragma once

#include "doorgame/animate.hpp"
#include "doorgame/storage.hpp"

namespace doorgame {

class Monster : public Storage, public Animate {
public:
    Monster(size_t loc);
};

using Monsters = std::vector<Monster>;

using MonsterList = std::vector<size_t>;

}  // namespace doorgame