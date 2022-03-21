#pragma once

#include <iostream>
#include <variant>

#include "doorgame/directions.hpp"
#include "doorgame/items.hpp"
#include "doorgame/targets.hpp"

namespace doorgame {

using Parameter = std::variant<std::monostate, Direction, Item, Target>;

std::ostream& operator<<(std::ostream& os, Parameter p);

};  // namespace doorgame