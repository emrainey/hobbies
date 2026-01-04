#include "raytrace/lights/spot.hpp"
#include "raytrace/mapping.hpp"

namespace raytrace {
namespace lights {
spot::spot(raytrace::ray const& r, raytrace::color const& C, precision intensity, iso::degrees const& incoming_angle)
    : light{r.location(), C, intensity, 1u, Falloff::InverseSquare}
    , m_direction{r.direction()}
    , m_incoming_angle{incoming_angle}
    , m_random_device{}
    , m_generator(m_random_device())
    , m_distribution{0, limit - 1} {
    basal::exception::throw_unless(incoming_angle.value > 0, __FILE__, __LINE__, "Must be positive angle.");
}

spot::spot(raytrace::ray&& r, raytrace::color const& C, precision intensity, iso::degrees const& incoming_angle)
    : light{std::move(r.location()), C, intensity, 1u, Falloff::InverseSquare}
    , m_direction{std::move(r.direction())}
    , m_incoming_angle{std::move(incoming_angle)}
    , m_random_device{}
    , m_generator(m_random_device())
    , m_distribution{0, limit - 1} {
    basal::exception::throw_unless(incoming_angle.value > 0, __FILE__, __LINE__, "Must be positive angle.");
}

precision spot::intensity_at(point const& world_point) const {
    // with spotlights, we draw the direction vector differently.
    // we're going to measure the angle between the direction of the spotlight
    // and the direction of the world point.
    raytrace::vector world_direction = world_point - position();
    iso::radians rad_angle = angle(m_direction, world_direction);
    iso::degrees deg_angle;
    iso::convert(deg_angle, rad_angle);
    if (deg_angle <= m_incoming_angle) {
        return light::intensity_at(world_point);  // use parent logic
    } else {
        return 0.0_p;
    }
}

ray spot::incident(point const& world_point, size_t sample_index __attribute__((unused))) const {
    return ray(world_point, position() - world_point);
}

void spot::print(std::ostream& os, char const str[]) const {
    os << " spot @" << this << " " << str << " pointing: " << m_direction << ", " << m_color << std::endl;
}

ray spot::emit() {
    // find a point on golden ratio sphere. We'll test each one to see if it's in the right direction.
    vector unit;
    iso::degrees deg_angle;
    do {
        size_t index = static_cast<size_t>(m_distribution(m_generator));
        point pnt = raytrace::mapping::golden_ratio_mapper(index, limit);
        unit = (pnt - R3::origin);  // these don't need to be normalized since they are from a unit sphere.
        iso::radians rad_angle = angle(m_direction, unit);
        iso::convert(deg_angle, rad_angle);
    } while (deg_angle > m_incoming_angle);
    statistics::get().emitted_rays++;
    return ray(position(), unit);
}

}  // namespace lights
}  // namespace raytrace
