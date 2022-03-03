#include "raytrace/mediums/conductor.hpp"

namespace raytrace {

namespace mediums {

conductor::conductor() : medium() {
    m_transmissivity = 0.05;  // all light is bounced, any transmitted light is absorbed immediately within nanometers
}

color conductor::bounced(const raytrace::point& volumetic_point, const color& incoming) const {
    using namespace operators;
    // conductors alter the color of the light bouncing off them
    return m_diffuse * incoming;
}

}  // namespace mediums

}  // namespace raytrace
