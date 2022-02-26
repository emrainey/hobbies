#include "raytrace/lights/bulb.hpp"
#include "raytrace/mapping.hpp"
#include "raytrace/laws.hpp"

namespace raytrace {
namespace lights {

using namespace linalg::operators;

bulb::bulb(const point& P, element_type radius, const color& C, element_type intensity, size_t samples)
    : light(C, intensity, samples)
    , entity(P)
    , m_radius(radius)
    {}

bulb::bulb(point&& P, element_type radius, const color& C, element_type intensity, size_t samples)
    : light(C, intensity, samples)
    , entity(std::move(P))
    , m_radius(radius)
    {}

element_type bulb::intensity_at(const point& world_point) const {
    using namespace geometry::operators;
    vector direction = position() - world_point;
    element_type d = direction.magnitude();
    if (basal::equals_zero(d)) {
        return m_intensity;
    } else {
        return m_intensity / (d * d);
    }
}

ray bulb::incident(const point& world_point, size_t sample_index) const {
    // we'll get a perturbation vector from the golden_ratio_mapper
    raytrace::vector perturb = raytrace::mapping::golden_ratio_mapper(sample_index, m_samples) - geometry::R3::origin;
    raytrace::vector shadow = position() - world_point;
    //if (dot(perturb, shadow) > 0.0) { // check for visibility
        // if the dot is positive, the point is on the non-visible side of the sphere
        // reflect it to the visible side with the double cross
    //    perturb = laws::reflection(cross(cross(perturb, shadow), shadow), perturb);
    //}
    perturb *= m_radius;
    return ray(world_point, shadow + perturb);
}

void bulb::print(const char str[]) const {
    std::cout << str << " bulb:" << m_samples << " @" << this << " " << position() << ", " << m_color << std::endl;
}

} // namespace lights

std::ostream& operator<<(std::ostream& os, const lights::bulb& l) {
    os << " bulb:" << l.number_of_samples() << " " << l.position() << ", " << l.color_at(l.position());
    return os;
}

} // namespace raytrace
