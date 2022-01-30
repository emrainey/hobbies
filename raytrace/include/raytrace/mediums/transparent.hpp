#pragma once
#include "raytrace/mediums/dielectric.hpp"

namespace raytrace {

namespace mediums {

/** A class of mediums which are dominated by largely transmitted rays, like glass, etc */
class transparent : public dielectric {
public:
    /** The medium's refractive index and fade factor */
    transparent(element_type eta, element_type fade);
    virtual ~transparent() = default;

    /** @copydoc medium::radiosity */
    void radiosity( const raytrace::point& volumetric_point,
                    element_type refractive_index,
                    const iso::radians& incident_angle,
                    const iso::radians& transmitted_angle,
                    element_type& emitted,
                    element_type& reflected,
                    element_type& transmitted) const override;

    /** @copydoc medium::absorbance */
    element_type absorbance(element_type distance) const override;
protected:
    element_type m_fade;
};

/** These are all measured for 589nm light */
namespace refractive_index {
constexpr static element_type vaccum     = 1.0;
constexpr static element_type air        = 1.000293;  // at 1 ATM and at 0 deg.
constexpr static element_type water      = 1.333;     // at 1 ATM and at 20 deg
constexpr static element_type oil        = 1.47;      // olive oil
constexpr static element_type ice        = 1.31;
constexpr static element_type quartz     = 1.46;
constexpr static element_type glass      = 1.52;
constexpr static element_type lexan      = 1.58;
constexpr static element_type sapphire   = 1.77;
constexpr static element_type zirconia   = 2.15;
constexpr static element_type diamond    = 2.42;
constexpr static element_type moissanite = 2.65;

/** Computes the dispersion of a specific type of dielectric material at an exact wavelength of light.
 * High quality SCHOTT glass and others can use this formula */
inline double dispersion(double a1, double a2, double b1, double b2, double c1, double c2, double w_nm) {
    double w_um = w_nm * 0.001;
    double w2 = w_um * w_um;
    double n2 = ((a1*w2) / (w2 - a2)) +
                ((b1*w2) / (w2 - b2)) +
                ((c1*w2) / (w2 - c2)) + 1.0;
    return std::sqrt(n2);
}

} // refractive_index


/** The common vaccum, perfectly transparent with no fade */
const transparent vaccum(refractive_index::vaccum, 0.0);

} // namespace mediums

} // namespace raytrace
