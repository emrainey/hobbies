/**
 * @file
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include "doorgame/damages.hpp"
#include "doorgame/health.hpp"

namespace doorgame {

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
    std::ostream& stream(std::ostream& os) const;

protected:
    Health m_health;
    size_t m_location;
};

}  // namespace doorgame