#include "raytrace/lights/spot.hpp"

namespace raytrace {
namespace lights {
spot::spot(const raytrace::ray& r, const raytrace::color& C, element_type intensity, const iso::degrees& incoming_angle)
    : light{C, intensity, 1}, entity{r.location()}, m_direction{r.direction()}, m_incoming_angle{incoming_angle} {
    basal::exception::throw_unless(incoming_angle.value > 0, __FILE__, __LINE__, "Must be positive angle.");
}

spot::spot(raytrace::ray&& r, const raytrace::color& C, element_type intensity, const iso::degrees& incoming_angle)
    : light{C, intensity, 1}
    , entity{std::move(r.location())}
    , m_direction{std::move(r.direction())}
    , m_incoming_angle{std::move(incoming_angle)} {
    basal::exception::throw_unless(incoming_angle.value > 0, __FILE__, __LINE__, "Must be positive angle.");
}

element_type spot::intensity_at(const point& world_point) const {
    // with spotlights, we draw the direction vector differently.
    // we're going to measure the angle between the direction of the spotlight
    // and the direction of the world point.
    raytrace::vector world_direction = world_point - position();
    iso::radians rad_angle = angle(m_direction, world_direction);
    iso::degrees deg_angle;
    iso::convert(deg_angle, rad_angle);
    if (deg_angle <= m_incoming_angle) {
        element_type d = world_direction.magnitude();
        if (basal::equals_zero(d)) {  // prevent divide by zero
            return m_intensity;       // full intensity, not inf
        } else {
            // using square fall-off rule
            return m_intensity / (d * d);
        }
    } else {
        return 0.0;
    }
}

ray spot::incident(const point& world_point, size_t sample_index __attribute__((unused))) const {
    return ray(world_point, position() - world_point);
}

void spot::print(const char str[]) const {
    std::cout << str << " spot @" << this << " pointing: " << m_direction << ", " << m_color << std::endl;
}

}  // namespace lights

std::ostream& operator<<(std::ostream& os, const lights::spot& l) {
    os << " spot " << l.incident(geometry::R3::origin, 0) << " " << l.color_at(geometry::R3::origin);
    return os;
}

}  // namespace raytrace
