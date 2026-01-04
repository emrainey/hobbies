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
    // if (dot(perturb, shadow) > 0.0_p) { // check for visibility
    //  if the dot is positive, the point is on the non-visible side of the sphere
    //  reflect it to the visible side with the precision cross
    //    perturb = laws::reflection(cross(cross(perturb, shadow), shadow), perturb);
    //}
    perturb *= m_radius;
    return ray(world_point, shadow + perturb);
}

void bulb::print(std::ostream& os, char const str[]) const {
    os << " bulb @ " << this << " " << str << " " << m_samples << " @" << this << " " << position() << ", " << m_color
       << std::endl;
}

ray bulb::emit() {
    // the limit to the number of directions we can emit from
    static const size_t limit = 1024u;
    static std::random_device rd;
    static std::mt19937 generator{rd()};
    static std::uniform_int_distribution<> distribution(0, limit - 1);
    size_t s = static_cast<size_t>(distribution(generator));
    // Emit a ray in a random direction
    point pnt = raytrace::mapping::golden_ratio_mapper(s, limit);
    vector dir = (pnt - R3::origin);  // these don't need to be normalized since they are from a unit sphere.
    statistics::get().emitted_rays++;
    return ray(position(), dir);
}

}  // namespace lights
}  // namespace raytrace
