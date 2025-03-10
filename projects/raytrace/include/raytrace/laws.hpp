#include <geometry/geometry.hpp>

#include "raytrace/types.hpp"
#include "raytrace/mapping.hpp"

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

/// Computes a random vector from the point of intersection to a point on a sphere based off the normal (N).
/// @param N The normal of the surface, normalized.
/// @param I The incident point
/// @retval A vector pointing to a random point on the sphere, not normalized.
inline vector lambertian(vector const& N, point const& I) {
    // pick a random number in a power of two range so we don't have to use %, instead use &
    size_t const range = (1U << 16U);
    size_t const value = static_cast<size_t>(rand() & static_cast<int>(range - 1U));
    // use the golden ratio mapper to find a point on the unit sphere then make it a vector from the unit center
    raytrace::vector P = (mapping::golden_ratio_mapper(value, range) - geometry::R3::origin);
    // since the sphere is a sphere we don't need to rotate or scale the point
    // translate the point from the unit sphere to the sphere at the point of intersection
    // by computing the vector from the incidence point to the point on the sphere.
    raytrace::point C = I + N;   // center of the sphere
    raytrace::point S = C + P;   // the point on the sphere.
    raytrace::vector R = S - I;  // the vector from the incidence point to the point on the sphere.

    // we just need to be cautious about points at or near the point of incidence
    // since that can cause zero length vectors.
    if (R.quadrance() < basal::smallish) {
        // in this case, we just return the normal.
        return N;
    } else {
        return R;
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
inline precision fresnel(precision n1, precision n2, iso::radians const& theta_i, iso::radians const& theta_tr) {
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
    // R(theta) = R0 + (1 - R0)(1 - cos(theta))^5
    // R0 = ((n1 - n2) / (n1 + n2))^2
    precision r0 = (n1 - n2) / (n1 + n2);
    r0 *= r0;
    precision one_minus_cos = 1.0_p - std::cos(theta.value);
    return r0 + (1.0_p - r0) * one_minus_cos * one_minus_cos * one_minus_cos * one_minus_cos * one_minus_cos;
}

/// Computes the Cauchy Principal Value of a function given two values and the frequency of light (not the wavelength).
/// @param a The first value
/// @param b The second value
/// @param light The frequency of the light
inline precision cauchy(precision a, precision b, iso::hertz light) {
    auto wavelength = 1.0f / light.value;
    return a + (b / (wavelength * wavelength));
}

///
/// Computes the penetration depth of light of a any material
/// @param permeability (mu) The magnetic permeability of the material
/// @param permittivity (epsilon) The electric permittivity of the material
/// @param resistivity (rho) The resistivity of the material (inverse of conductivity)
/// @param frequency The cycles per second of the light.
/// @return The depth in meters?
/// @see https://en.wikipedia.org/wiki/Skin_effect
inline precision penetration_depth(precision permeability, precision permittivity, precision resistivity,
                                   iso::hertz frequency) {
    // omega = 2 * pi * f
    precision omega = 2 * iso::pi * frequency.value;
    // rwe = rho * omega * epsilon
    precision rho_omega_epsilon = resistivity * omega * permittivity;
    // 2 * rho / omega * mu
    precision two_rho_over_omega_mu = (2 * resistivity) / (omega * permeability);
    // inner square root of (rwe^2 + 1)
    auto r = sqrt((rho_omega_epsilon * rho_omega_epsilon) + 1);
    // final square root of (2 * rho / omega * mu * (r + rwe))
    return sqrt(two_rho_over_omega_mu * (r + rho_omega_epsilon));
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

/// Computes the inverse square of distance. At zero distance, the values is 1.0 and drop below 1 as distance increases.
/// Throws an exception if the distance is less than zero.
inline precision inverse_square(precision distance) {
    throw_exception_if(distance < 0.0_p, "Distance %lf must be greater than or equal to zero.", distance);
    precision const b = distance + 1.0_p;  // to avoid division by zero and infinite values.
    return 1.0_p / (b * b);
}

}  // namespace laws

}  // namespace raytrace