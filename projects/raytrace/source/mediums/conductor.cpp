#include "raytrace/mediums/conductor.hpp"

namespace raytrace {

namespace mediums {

conductor::conductor() : medium{} {
    m_reflectivity = 0.95_p;    // all light is bounced, any transmitted light is absorbed immediately within nanometers
    m_transmissivity = 0.05_p;  // most light does not continue once transmitted
}

color conductor::bounced(raytrace::point const& volumetric_point, color const& incoming) const {
    using namespace operators;
    (void)volumetric_point;
    // conductors alter the color of the light bouncing off them
    return m_diffuse * incoming;
}

}  // namespace mediums

}  // namespace raytrace
