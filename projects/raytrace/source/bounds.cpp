#include <raytrace/raytrace.hpp>

namespace raytrace {

point Bounds::center() const {
    auto vmin = min - R3::origin;
    auto vmax = max - R3::origin;
    auto v = (vmin + vmax) / 2.0;
    return R3::origin + v;
}

/// Determines if a point is within the bounds
bool Bounds::contained(point const& p) const {
    return (min.x <= p.x and p.x <= max.x and min.y <= p.y and p.y <= max.y and min.z <= p.z and p.z <= max.z);
}

/// Determines if a ray intersects the bounds
bool Bounds::intersects(ray const& r) const {
    precision const& x = r.location().x;
    precision const& y = r.location().y;
    precision const& z = r.location().z;
    precision const& i = r.direction()[0];
    precision const& j = r.direction()[1];
    precision const& k = r.direction()[2];
    if (not basal::is_exactly_zero(i)) {
        precision t_min = (min.x - x) / i;
        if (t_min >= 0.0_p) {
            auto point_min = r.distance_along(t_min);
            if (contained(point_min)) {
                return true;
            }
        }
        precision t_max = (max.x - x) / i;
        if (t_max >= 0.0_p) {
            auto point_max = r.distance_along(t_max);
            if (contained(point_max)) {
                return true;
            }
        }
    }
    if (not basal::is_exactly_zero(j)) {
        precision t_min = (min.y - y) / j;
        if (t_min >= 0.0_p) {
            auto point_min = r.distance_along(t_min);
            if (contained(point_min)) {
                return true;
            }
        }
        precision t_max = (max.y - y) / j;
        if (t_max >= 0.0_p) {
            auto point_max = r.distance_along(t_max);
            if (contained(point_max)) {
                return true;
            }
        }
    }
    if (not basal::is_exactly_zero(k)) {
        precision t_min = (min.z - z) / k;
        if (t_min >= 0.0_p) {
            auto point_min = r.distance_along(t_min);
            if (contained(point_min)) {
                return true;
            }
        }
        precision t_max = (max.z - z) / k;
        if (t_max >= 0.0_p) {
            auto point_max = r.distance_along(t_max);
            if (contained(point_max)) {
                return true;
            }
        }
    }
    return false;
}

/// Determines if another bounds intersects with this bounds.
bool Bounds::intersects(Bounds const& b) const {
    return ((min.x <= b.max.x and b.min.x <= max.x) and
            (min.y <= b.max.y and b.min.y <= max.y) and
            (min.z <= b.max.z and b.min.z <= max.z));
}

bool Bounds::is_infinite() const {
    return (std::isnan(min.x) or std::isnan(min.y) or std::isnan(min.z) or std::isnan(max.x) or std::isnan(max.y)
            or std::isnan(max.z));
}

void Bounds::grow(Bounds const& b) {
    min.x = std::min(min.x, b.min.x);
    min.y = std::min(min.y, b.min.y);
    min.z = std::min(min.z, b.min.z);
    max.x = std::max(max.x, b.max.x);
    max.y = std::max(max.y, b.max.y);
    max.z = std::max(max.z, b.max.z);
}

} // namespace raytrace
