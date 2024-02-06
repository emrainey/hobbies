/// @file
/// The Event Type
/// @copyright Copyright (c) 2022
#pragma once

#include <tuple>

#include "doorgame/actions.hpp"
#include "doorgame/directions.hpp"
#include "doorgame/items.hpp"
#include "doorgame/parameters.hpp"
#include "doorgame/targets.hpp"

namespace doorgame {

/// Events are essentially a combination of Subject, Verb, Object, Modifier
using Event = std::tuple<Target, Action, Target, Parameter>;

}  // namespace doorgame