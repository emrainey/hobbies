#include "raytrace/mediums/conductor.hpp"

namespace raytrace {

namespace mediums {

conductor::conductor() : medium() {
    m_transmissivity = 0.05; // all light is bounced, any transmitted light is absorbed immediately within nanometers
}

} // namespace mediums

} // namespace raytrace
