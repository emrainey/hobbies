#pragma once

#include <iostream>
#include <variant>

#include "doorgame/damages.hpp"
#include "doorgame/directions.hpp"
#include "doorgame/health.hpp"
#include "doorgame/items.hpp"

namespace doorgame {

using Parameter = std::variant<std::monostate, Direction, Item, Damage, Health>;
//                             0,              1,         2,    3,      4

};  // namespace doorgame