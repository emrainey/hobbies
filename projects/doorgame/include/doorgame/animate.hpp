/// @file
/// @copyright Copyright (c) 2022

#pragma once

#include "doorgame/damages.hpp"
#include "doorgame/health.hpp"

namespace doorgame {

/// An Animate object is a living entity in the game.
class Animate {
public:
    Animate(size_t location);
    bool is_alive() const;
    Health get_health() const;
    void location(size_t newloc);
    size_t location() const;
    void take(Damage amount);
    void resurrect();
    void give_healing(size_t amount);
    Animate const& as_animate() const;
    Animate& as_animate();

protected:
    Health m_health;
    size_t m_location;
};

}  // namespace doorgame