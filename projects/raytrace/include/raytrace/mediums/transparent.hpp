#pragma once
#include "raytrace/mediums/dielectric.hpp"

namespace raytrace {

namespace mediums {

/// A class of mediums which are dominated by largely transmitted rays, like glass, etc
class transparent : public dielectric {
public:
    /// The medium's refractive index and fade factor
    transparent(precision eta, precision fade, const color& diffuse);
    virtual ~transparent() = default;

    /// @copydoc medium::radiosity
    void radiosity(const raytrace::point& volumetric_point, precision refractive_index,
                   const iso::radians& incident_angle, const iso::radians& transmitted_angle, precision& emitted,
                   precision& reflected, precision& transmitted) const override;

    /// @copydoc medium::absorbance
    color absorbance(precision distance, const color& given_color) const override;

protected:
    precision m_fade;
};

/// These are all measured for 589nm light
namespace refractive_index {
constexpr static precision vaccum = 1.0_p;
constexpr static precision air = 1.000293_p;  // at 1 ATM and at 0 deg.
constexpr static precision water = 1.333_p;   // at 1 ATM and at 20 deg
constexpr static precision oil = 1.47_p;      // olive oil
constexpr static precision ice = 1.31_p;
constexpr static precision quartz = 1.46_p;
constexpr static precision glass = 1.52_p;
constexpr static precision lexan = 1.58_p;
constexpr static precision sapphire = 1.77_p;
constexpr static precision zirconia = 2.15_p;
constexpr static precision diamond = 2.42_p;
constexpr static precision moissanite = 2.65_p;

/// Computes the dispersion of a specific type of dielectric material at an exact wavelength of light.
/// High quality SCHOTT glass and others can use this formula */
inline precision dispersion(precision a1, precision a2, precision b1, precision b2, precision c1, precision c2, precision w_nm) {
    precision w_um = w_nm * 0.001_p;
    precision w2 = w_um * w_um;
    precision n2 = ((a1 * w2) / (w2 - a2)) + ((b1 * w2) / (w2 - b2)) + ((c1 * w2) / (w2 - c2)) + 1.0_p;
    return std::sqrt(n2);
}

}  // namespace refractive_index

/// The common vaccum, perfectly transparent with no fade
const transparent vaccum(refractive_index::vaccum, 0.0_p, colors::white);

/// The normal atmosphere on earth
const transparent earth_atmosphere(refractive_index::air, 0.004_p, colors::light_blue);

}  // namespace mediums

}  // namespace raytrace
