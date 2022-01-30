#pragma once
#include "raytrace/medium.hpp"

namespace raytrace {

namespace mediums {

/** A conductor is a "shiny" metal which has high reflectivity and very low transmitted light. Can be non-shiny and rough as well. */
class conductor : public medium {
public:
    conductor();
    virtual ~conductor() = default;
};

} // namespace mediums

} // namespace raytrace
