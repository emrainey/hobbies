#include "raytrace/lights/spot.hpp"

namespace raytrace {
namespace lights {
spot::spot(raytrace::ray const& r, raytrace::color const& C, precision intensity, iso::degrees const& incoming_angle)
    : light{r.location(), C, intensity, 1u, Falloff::InverseSquare}
    , m_direction{r.direction()}
    , m_incoming_angle{incoming_angle} {
    basal::exception::throw_unless(incoming_angle.value > 0, __FILE__, __LINE__, "Must be positive angle.");
}

spot::spot(raytrace::ray&& r, raytrace::color const& C, precision intensity, iso::degrees const& incoming_angle)
    : light{std::move(r.location()), C, intensity, 1u, Falloff::InverseSquare}
    , m_direction{std::move(r.direction())}
    , m_incoming_angle{std::move(incoming_angle)} {
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

void spot::print(char const str[]) const {
    std::cout << str << " spot @" << this << " pointing: " << m_direction << ", " << m_color << std::endl;
}

}  // namespace lights

std::ostream& operator<<(std::ostream& os, lights::spot const& l) {
    os << " spot " << l.incident(geometry::R3::origin, 0) << " " << l.color_at(geometry::R3::origin);
    return os;
}

}  // namespace raytrace
