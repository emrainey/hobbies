#pragma once

#include "raytrace/mediums/medium.hpp"

namespace raytrace {

namespace mediums {

/// A conductor is a "shiny" metal which has high reflectivity and very low transmitted light. Can be non-shiny and
/// rough as well. */
class conductor : public medium {
public:
    conductor();

    // bounced light will be altered by conductors
    color bounced(raytrace::point const& volumetric_point, color const& incoming) const override;

    virtual ~conductor() = default;
};

}  // namespace mediums

}  // namespace raytrace
