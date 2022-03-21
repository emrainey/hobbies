#pragma once

#include "doorgame/animate.hpp"
#include "doorgame/storage.hpp"

namespace doorgame {

class Player
    : public Storage
    , public Animate {
public:
    Player(size_t loc);
    std::ostream& stream(std::ostream& os) const;
};

}  // namespace doorgame
