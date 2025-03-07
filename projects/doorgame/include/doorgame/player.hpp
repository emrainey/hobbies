#pragma once

#include "doorgame/animate.hpp"
#include "doorgame/storage.hpp"

namespace doorgame {

class Player : public Storage, public Animate {
public:
    Player(size_t loc);
};

}  // namespace doorgame
