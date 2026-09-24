#include "raytrace/clouds/CloudIntegrator.hpp"

namespace raytrace {

namespace clouds {

CloudIntegrator::CloudIntegrator(CloudLayer const& layer, precision step_size) : m_layer{layer}, m_step{step_size} {
    basal::exception::throw_if(basal::nearly_zero(m_step), __FILE__, __LINE__, "Step size can't be zero");
}

precision CloudIntegrator::step_size() const {
    return m_step;
}

bool CloudIntegrator::clip(ray const& r, precision& t0, precision& t1) const {
    // The standard slab test for an axis aligned box, worked in ray parameters t
    // where world = location + t * direction.
    Bounds const box = m_layer.bounds();
    point const& loc = r.location();
    vector const& dir = r.direction();

    precision near_t = basal::neg_inf;
    precision far_t = basal::pos_inf;

    // x
    if (basal::is_exactly_zero(dir.x())) {
        if (loc.x() < box.min.x() or loc.x() > box.max.x()) {
            return false;
        }
    } else {
        precision lo = (box.min.x() - loc.x()) / dir.x();
        precision hi = (box.max.x() - loc.x()) / dir.x();
        if (lo > hi) {
            std::swap(lo, hi);
        }
        near_t = std::max(near_t, lo);
        far_t = std::min(far_t, hi);
    }
    // y
    if (basal::is_exactly_zero(dir.y())) {
        if (loc.y() < box.min.y() or loc.y() > box.max.y()) {
            return false;
        }
    } else {
        precision lo = (box.min.y() - loc.y()) / dir.y();
        precision hi = (box.max.y() - loc.y()) / dir.y();
        if (lo > hi) {
            std::swap(lo, hi);
        }
        near_t = std::max(near_t, lo);
        far_t = std::min(far_t, hi);
    }
    // z
    if (basal::is_exactly_zero(dir.z())) {
        if (loc.z() < box.min.z() or loc.z() > box.max.z()) {
            return false;
        }
    } else {
        precision lo = (box.min.z() - loc.z()) / dir.z();
        precision hi = (box.max.z() - loc.z()) / dir.z();
        if (lo > hi) {
            std::swap(lo, hi);
        }
        near_t = std::max(near_t, lo);
        far_t = std::min(far_t, hi);
    }

    near_t = std::max(near_t, 0.0_p);
    if (near_t > far_t) {
        return false;
    }
    t0 = near_t;
    t1 = far_t;
    return true;
}

bool CloudIntegrator::intersects(ray const& r) const {
    precision t0 = 0.0_p, t1 = 0.0_p;
    return clip(r, t0, t1);
}

precision CloudIntegrator::optical_depth_between(point const& a, point const& b) const {
    vector span = b - a;
    precision length = span.magnitude();
    if (basal::nearly_zero(length)) {
        return 0.0_p;
    }
    // Align the sampling to exactly tile the segment so a constant density recovers
    // Beer-Lambert (sum rho * ds == rho * length).
    size_t steps = static_cast<size_t>(std::ceil(length / m_step));
    steps = std::max<size_t>(1, steps);
    precision ds = length / static_cast<precision>(steps);
    precision accumulated = 0.0_p;
    for (size_t i = 0; i < steps; i++) {
        precision fraction = (static_cast<precision>(i) + 0.5_p) / static_cast<precision>(steps);
        point sample = a + (span * fraction);
        accumulated += m_layer.density(sample) * ds;
    }
    return accumulated;
}

precision CloudIntegrator::optical_depth(ray const& r) const {
    precision t0 = 0.0_p, t1 = 0.0_p;
    if (not clip(r, t0, t1)) {
        return 0.0_p;
    }
    return optical_depth_between(r.distance_along(t0), r.distance_along(t1));
}

precision CloudIntegrator::transmittance(ray const& r) const {
    return std::exp(-m_layer.extinction() * optical_depth(r));
}

precision CloudIntegrator::brightening(ray const& r, vector const& light_direction) const {
    precision t0 = 0.0_p, t1 = 0.0_p;
    if (not clip(r, t0, t1)) {
        return 0.0_p;
    }
    point entry = r.distance_along(t0);
    point exit = r.distance_along(t1);
    vector span = exit - entry;
    precision length = span.magnitude();
    if (basal::nearly_zero(length)) {
        return 0.0_p;
    }
    // The optical path from a point back to the light-facing boundary.
    vector toward_light = -(light_direction.normalized());

    size_t steps = static_cast<size_t>(std::ceil(length / m_step));
    steps = std::max<size_t>(1, steps);
    precision summed = 0.0_p;
    for (size_t i = 0; i < steps; i++) {
        precision fraction = (static_cast<precision>(i) + 0.5_p) / static_cast<precision>(steps);
        point sample = entry + (span * fraction);
        ray light_ray{sample, toward_light};
        precision tau_light = optical_depth(light_ray);
        // Single scattering: only where there is cloud (density) to scatter the
        // un-extinguished light arriving at the sample.
        summed += m_layer.albedo() * m_layer.density(sample) * std::exp(-m_layer.extinction() * tau_light);
    }
    return std::clamp(summed / static_cast<precision>(steps), 0.0_p, m_layer.albedo());
}

}  // namespace clouds

}  // namespace raytrace