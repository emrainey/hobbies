#pragma once

#include "doorgame/animate.hpp"
#include "doorgame/storage.hpp"

namespace doorgame {

class Monster
    : public Storage
    , public Animate {
public:
    Monster(size_t loc);
    std::ostream& stream(std::ostream& os) const;
};

using Monsters = std::vector<Monster>;

}