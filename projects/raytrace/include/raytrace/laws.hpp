#include <geometry/geometry.hpp>

#include "raytrace/types.hpp"

namespace raytrace {

/// The common laws of physics used in Raytracers adapted to return values in new vectors instead of in angles.
namespace laws {

///
/// Computes Snell's law from a given set of "n" (eta in greek) or refractive indexes
/// and the normalized normal of the surface
/// @param N The normal of the surface, normalized.
/// @warning Must always point oppositely the incident vector
/// @param I The incident vector to the surface, normalized.
/// @warning Must always point oppositely the normal vector
/// @param eta1 The refractive index of the current medium
/// @param eta2 The refractive index of the next medium
/// @retval R3::null when total internal reflection has occurred.
///
inline vector snell(vector const& N, vector const& I, precision eta1, precision eta2) {
    // Snell's Law here is about the equality eta1 * sin(theta) = eta2 * sin(phi)
    // sin(phi) = eta1/eta2 * sin(theta)
    // we want the value of phi as applied on the outgoing vector
    precision eta = eta1 / eta2;
    precision eta_eta = eta * eta;
    precision cos_theta = dot(N, I);
    precision sin2_phi = eta_eta * (1.0_p - (cos_theta * cos_theta));
    if (sin2_phi <= 1.0_p) {
        return (eta * I) - (((eta * cos_theta) + sqrt(1.0_p - sin2_phi)) * N);
    } else {
        return geometry::R3::null;
    }
}

/// Computes the "perfect" reflection vector from a Normal and Incident.
inline vector reflection(vector const& N, vector const& I) {
    // this scales the normal to the incident angle size, precisions that and subtracts that
    // from the normalized incident to essentially flip it around the plane of the normal.
    // r = I - 2*dot(nI,N)*N
    precision cos_theta = dot(N, I);
    return (I - (2 * (cos_theta * N)));
}

/// Computes the Fresnel Reflectance of a dielectric given the etas and angles.
/// @param n1 The refractive index of the medium being exitted
/// @param n2 The refractive index of the medium being entered
/// @param theta_i The incident angle to the normal
/// @param theta_tr The refracted angle to the normal
/// @return The coefficient of reflectance. The coefficient of refraction is 1 - reflectance.
///
inline precision fresnel(precision n1, precision n2, iso::radians const& theta_i,
                            iso::radians const& theta_tr) {
    precision cos_theta_i = std::cos(theta_i.value);
    precision cos_theta_t = std::cos(theta_tr.value);
    precision Rs = (n1 * cos_theta_i - n2 * cos_theta_t) / (n1 * cos_theta_i + n2 * cos_theta_t);
    Rs *= Rs;
    precision Rp = (n2 * cos_theta_i - n1 * cos_theta_t) / (n2 * cos_theta_i + n1 * cos_theta_t);
    Rp *= Rp;
    return (0.5_p * (Rs + Rp));
}

/// Approximates the coefficient of specular reflection for a dielectric given the medium interfaces and angle from the
/// normal.
/// @param n1 The refractive index of the medium being exitted
/// @param n2 The refractive index of the medium being entered
/// @param theta the angle between the Normal the the incident light.
///
inline precision schlicks(precision n1, precision n2, iso::radians const& theta) {
    precision r0 = (n1 - n2) / (n1 + n2);
    r0 *= r0;
    precision one_minus_cos = 1.0_p - std::cos(theta.value);
    return r0 + (1.0_p - r0) * one_minus_cos * one_minus_cos * one_minus_cos * one_minus_cos * one_minus_cos;
}

///
/// Computes the penetration depth of light of a any material
/// @param permeability The magnetic permeability of the material
/// @param permittivity The electric permittivity of the material
/// @param resistivity The resistivity of the material
/// @param frequency The cycles per second of the light.
/// @return The depth in meters?
///
inline precision penetration_depth(precision permeability, precision permittivity, precision resistivity,
                                      precision frequency) {
    precision omega = 2 * iso::pi * frequency;  // FIXME should be in iso::radians or iso::????
    precision rho_omega_eta = resistivity * omega * permittivity;
    precision two_omega_mu = 2 * omega * permeability;
    return sqrt(resistivity / two_omega_mu) * sqrt(sqrt((rho_omega_eta * rho_omega_eta) + 1) + rho_omega_eta);
}

///
/// Computes the diminishment of the intensity of light through a medium with a given distance and coefficient
/// @param D the distance vector to the next change of medium
/// @param C The coefficient of dropoff in the medium. Values can be from 0-1 inclusive. Values under 0.2_p look more
/// transparent.
///
inline precision beers(vector const& D, precision C) {
    return std::exp(-D.magnitude() * C);
}
}  // namespace laws

}  // namespace raytrace