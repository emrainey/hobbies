#pragma once

#include "raytrace/types.hpp"

namespace raytrace {

namespace animation {
/// Defines a structure which anchors the camera attributes to a specific point at a specific time.
struct Attributes {
    raytrace::point from{0, 10, 10};  ///< The location where the camera is
    raytrace::point at{0, 0, 0};      ///< The location where the camera is looking
    iso::degrees fov{55.0_p};         ///< The field of view of the camera (horizontal)
};

/// Defines the mappers to use for the camera attributes. Defaults to linear maps.
struct Mappers {
    geometry::mapper from{
        geometry::mapping::linear};  ///< The interpolation function to use on the position of the camera
    geometry::mapper at{geometry::mapping::linear};  ///< The interpolation function to use on the look position of the
                                                     ///< camera (where it's looking)
    geometry::mapper fov{
        geometry::mapping::linear};  ///< The interpolation function to use on the field of view of the camera
};
struct Anchor {
    Attributes start;       ///< The starting camera location and orientation
    Attributes limit;       ///< The ending camera location and orientation
    Mappers mappers;        ///< The set of mapping functions to use to transition from the start to the limit
    iso::seconds duration;  ///< The length of time when the camera should transition from the start to the limit
};
using anchors = std::vector<Anchor>;

class Animator {
public:
    Animator(precision frame_rate, anchors const& a);

    /// Return true until the end of the animation
    explicit operator bool() const;

    /// @return The next set of camera attributes interpolated between the start and limit attributes.
    Attributes operator()();

protected:
    /// Interpolates between the start and limit attributes using the mappers and the delta time.
    Attributes interpolate(Attributes const& start, Attributes const& limit, Mappers const& mappers, precision dt);

    size_t index_{0};
    iso::seconds now_{0.0};
    iso::seconds start_{0.0};  ///< The start of the current anchor duration
    iso::seconds delta_{0.0};
    anchors const& anchors_;
};

}  // namespace animation

}  // namespace raytrace
