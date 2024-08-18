#include "raytrace/lights/bulb.hpp"

#include "raytrace/laws.hpp"
#include "raytrace/mapping.hpp"

namespace raytrace {
namespace lights {

using namespace linalg::operators;

bulb::bulb(point const& P, precision radius, color const& C, precision intensity, size_t samples)
    : light{P, C, intensity, samples, Falloff::InverseSquare}, m_radius{radius} {
}

bulb::bulb(point&& P, precision radius, color const& C, precision intensity, size_t samples)
    : light{std::move(P), C, intensity, samples, Falloff::InverseSquare}, m_radius{radius} {
}

ray bulb::incident(point const& world_point, size_t sample_index) const {
    // we'll get a perturbation vector from the golden_ratio_mapper
    raytrace::vector perturb = raytrace::mapping::golden_ratio_mapper(sample_index, m_samples) - geometry::R3::origin;
    raytrace::vector shadow = position() - world_point;
    // if (dot(perturb, shadow) > 0.0) { // check for visibility
    //  if the dot is positive, the point is on the non-visible side of the sphere
    //  reflect it to the visible side with the precision cross
    //    perturb = laws::reflection(cross(cross(perturb, shadow), shadow), perturb);
    //}
    perturb *= m_radius;
    return ray(world_point, shadow + perturb);
}

void bulb::print(char const str[]) const {
    std::cout << str << " bulb:" << m_samples << " @" << this << " " << position() << ", " << m_color << std::endl;
}

}  // namespace lights

std::ostream& operator<<(std::ostream& os, lights::bulb const& l) {
    os << " bulb:" << l.number_of_samples() << " " << l.position() << ", " << l.color_at(l.position());
    return os;
}

}  // namespace raytrace
