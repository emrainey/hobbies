#include <geometry/geometry.hpp>
#include "raytrace/types.hpp"

namespace raytrace {

/** The common laws of physics used in Raytracers adapted to return values in new vectors instead of in angles. */
namespace laws {

    /**
     * Computes Snell's law from a given set of "n" (eta in greek) or refractive indexes
     * and the normalized normal of the surface
     * @param N The normal of the surface, normalized
     * @param I The incident vector to the surface, normalized
     * @param eta1 The refractive index of the current medium
     * @param eta2 The refractive index of the next medium
     * @retval R3::null when total internal reflection has occurred.
     */
    inline vector snell(const vector& N, const vector& I, element_type eta1, element_type eta2) {
        // Snell's Law here is about the equality eta1 * sin(theta) = eta2 * sin(phi)
        // sin(phi) = eta1/eta2 * sin(theta)
        // we want the value of phi as applied on the outgoing vector
        element_type eta = eta1/eta2;
        element_type eta_eta = eta * eta;
        element_type cos_theta = dot(N, I); // doesn't matter if -I since it'll be squared
        element_type sin2_phi = eta_eta * (1.0 - (cos_theta * cos_theta));
        if (sin2_phi <= 1.0) {
            return (eta * I) - (((eta * cos_theta) + sqrt(1.0 - sin2_phi)) * N);
        } else {
            return geometry::R3::null;
        }
    }

    /** Computes the "perfect" reflection vector from a Normal and Incident. */
    inline vector reflection(const vector& N, const vector& I) {
        // this scales the normal to the incident angle size, doubles that and subtracts that
        // from the normalized incident to essentially flip it around the plane of the normal.
        // r = I - 2*dot(nI,N)*N
        element_type cos_theta = dot(N, I);
        return (I - (2 * (cos_theta * N)));
    }

    /** Computes the Fresnel Reflectance of a dielectric given the etas and angles.
     * @param n1 The refractive index of the medium being exitted
     * @param n2 The refractive index of the medium being entered
     * @param theta_i The incident angle to the normal
     * @param theta_tr The refracted angle to the normal
     * @return The coefficient of reflectance. The coefficient of refraction is 1 - reflectance.
    */
    inline element_type fresnel(element_type n1, element_type n2, const iso::radians& theta_i, const iso::radians& theta_tr) {
        element_type cos_theta_i = std::cos(theta_i.value);
        element_type cos_theta_t = std::cos(theta_tr.value);
        element_type Rs = (n1 * cos_theta_i - n2 * cos_theta_t) / (n1 * cos_theta_i + n2 * cos_theta_t);
        Rs *= Rs;
        element_type Rp = (n1 * cos_theta_t - n2 * cos_theta_i) / (n1 * cos_theta_t + n2 * cos_theta_i);
        Rp *= Rp;
        return (0.5 * (Rs + Rp));
    }

    /** Approximates the coefficient of specular reflection for a dielectric given the medium interfaces and angle from the normal.
     * @param n1 The refractive index of the medium being exitted
     * @param n2 The refractive index of the medium being entered
     * @param theta the angle between the Normal the the incident light.
     */
    inline element_type schlicks(element_type n1, element_type n2, const iso::radians& theta) {
        element_type r0 = (n1 - n2) / (n1 + n2);
        r0 *= r0;
        element_type one_minus_cos = 1.0 - std::cos(theta.value);
        return r0 + (1.0 - r0)*one_minus_cos*one_minus_cos*one_minus_cos*one_minus_cos*one_minus_cos;
    }

    /**
     * Computes the penetration depth of light of a any material
     * @param permeability The magnetic permeability of the material
     * @param permittivity The electric permittivity of the material
     * @param resistivity The resistivity of the material
     * @param frequency The cycles per second of the light.
     * @return The depth in meters?
     */
    inline element_type penetration_depth(element_type permeability, element_type permittivity, element_type resistivity, element_type frequency) {
        element_type omega = 2 * iso::pi * frequency; // FIXME should be in iso::radians or iso::????
        element_type rho_omega_eta = resistivity * omega * permittivity;
        element_type two_omega_mu = 2 * omega * permeability;
        return sqrt(resistivity / two_omega_mu) * sqrt(sqrt((rho_omega_eta * rho_omega_eta) + 1) + rho_omega_eta);
    }

    /**
     * Computes the diminishment of the intensity of light through a medium with a given distance and coefficient
     * @param D the distance vector to the next change of medium
     * @param C The coefficient of dropoff in the medium. Values can be from 0-1 inclusive. Values under 0.2 look more transparent.
     */
    inline element_type beers(const vector& D, element_type C) {
        return std::exp(-D.magnitude() * C);
    }
}

}