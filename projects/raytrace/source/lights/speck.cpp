#include "raytrace/lights/speck.hpp"
#include "raytrace/laws.hpp"

namespace raytrace {
namespace lights {
using namespace linalg::operators;

speck::speck(point const& P, color const& C, precision intensity) : light(P, C, intensity, 1u, Falloff::InverseSquare) {
}

speck::speck(point&& P, color const& C, precision intensity)
    : light(std::move(P), C, intensity, 1u, Falloff::InverseSquare) {
}

ray speck::incident(point const& world_point, size_t sample_index __attribute__((unused))) const {
    return ray(world_point, position() - world_point);
}

void speck::print(std::ostream& os, char const str[]) const {
    os << "speck @" << this << " " << str << " " << position() << ", " << m_color << std::endl;
}

ray speck::emit() {
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
