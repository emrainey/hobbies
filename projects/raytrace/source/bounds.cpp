#include <basal/ieee754.hpp>
#include <basal/exception.hpp>
#include <raytrace/raytrace.hpp>

namespace raytrace {

Bounds::Bounds()
    : min{basal::neg_inf, basal::neg_inf, basal::neg_inf}, max{basal::pos_inf, basal::pos_inf, basal::pos_inf} {
}

Bounds::Bounds(point const& min, point const& max) : min{min}, max{max} {
    basal::exception::throw_unless(min.x < max.x, __FILE__, __LINE__, "min.x %lf must be less than max.x %lf\r\n",
                                   min.x, max.x);
    basal::exception::throw_unless(min.y < max.y, __FILE__, __LINE__, "min.y %lf must be less than max.y %lf\r\n",
                                   min.y, max.y);
    basal::exception::throw_unless(min.z < max.z, __FILE__, __LINE__, "min.z %lf must be less than max.z %lf\r\n",
                                   min.z, max.z);
}

point Bounds::center() const {
    if (not is_infinite()) {
        auto vmin = min - R3::origin;
        auto vmax = max - R3::origin;
        auto v = (vmin + vmax) / 2.0_p;
        return R3::origin + v;
    } else {
        // if the bounds are infinite on both sides, then the center is origin R3::origin
        if (std::isinf(min.x) and std::isinf(min.y) and std::isinf(min.z) and std::isinf(max.x) and std::isinf(max.y)
            and std::isinf(max.z)) {
            return R3::origin;
        }
        // if the bounds are infinite on one side but not the other, then there can be no defined center.
        return point{basal::nan, basal::nan, basal::nan};
    }
}

bool Bounds::intersects(ray const& r) const {
    precision const x = r.location().x;
    precision const y = r.location().y;
    precision const z = r.location().z;
    if (contained(r.location())) {
        raytrace::statistics::get().intersections_with_bounds++;
        return true;
    }
    precision const i = r.direction()[0];
    precision const j = r.direction()[1];
    precision const k = r.direction()[2];
    if (not basal::is_exactly_zero(i)) {
        precision t_min = (min.x - x) / i;
        if (basal::is_greater_than_or_equal_to_zero(t_min)) {
            auto point_min = r.distance_along(t_min);
            if (contained(point_min)) {
                raytrace::statistics::get().intersections_with_bounds++;
                return true;
            }
        }
        precision t_max = (max.x - x) / i;
        if (basal::is_greater_than_or_equal_to_zero(t_max)) {
            auto point_max = r.distance_along(t_max);
            if (contained(point_max)) {
                raytrace::statistics::get().intersections_with_bounds++;
                return true;
            }
        }
    }
    if (not basal::is_exactly_zero(j)) {
        precision t_min = (min.y - y) / j;
        if (basal::is_greater_than_or_equal_to_zero(t_min)) {
            auto point_min = r.distance_along(t_min);
            if (contained(point_min)) {
                raytrace::statistics::get().intersections_with_bounds++;
                return true;
            }
        }
        precision t_max = (max.y - y) / j;
        if (basal::is_greater_than_or_equal_to_zero(t_max)) {
            auto point_max = r.distance_along(t_max);
            if (contained(point_max)) {
                raytrace::statistics::get().intersections_with_bounds++;
                return true;
            }
        }
    }
    if (not basal::is_exactly_zero(k)) {
        precision t_min = (min.z - z) / k;
        if (basal::is_greater_than_or_equal_to_zero(t_min)) {
            auto point_min = r.distance_along(t_min);
            if (contained(point_min)) {
                raytrace::statistics::get().intersections_with_bounds++;
                return true;
            }
        }
        precision t_max = (max.z - z) / k;
        if (basal::is_greater_than_or_equal_to_zero(t_max)) {
            auto point_max = r.distance_along(t_max);
            if (contained(point_max)) {
                raytrace::statistics::get().intersections_with_bounds++;
                return true;
            }
        }
    }
    return false;
}

/// Determines if another bounds intersects with this bounds.
bool Bounds::intersects(Bounds const& b) const {
    return ((min.x <= b.max.x and b.min.x < max.x) and (min.y <= b.max.y and b.min.y < max.y)
            and (min.z <= b.max.z and b.min.z < max.z));
}

bool Bounds::is_infinite() const {
    return (std::isinf(min.x) or std::isinf(min.y) or std::isinf(min.z) or std::isinf(max.x) or std::isinf(max.y)
            or std::isinf(max.z));
}

void Bounds::grow(Bounds const& b) {
    min.x = std::min(min.x, b.min.x);
    min.y = std::min(min.y, b.min.y);
    min.z = std::min(min.z, b.min.z);
    max.x = std::max(max.x, b.max.x);
    max.y = std::max(max.y, b.max.y);
    max.z = std::max(max.z, b.max.z);
}

std::vector<Bounds> Bounds::split() const {
    std::vector<Bounds> sub_bounds;
    raytrace::point mid{(min.x + max.x) / 2.0_p, (min.y + max.y) / 2.0_p, (min.z + max.z) / 2.0_p};
    // need to create all the min points and max points
    // mins are mixed between min and mid
    std::array<point, NumSubBounds> sub_min{
        point{min.x, min.y, min.z}, point{min.x, min.y, mid.z}, point{min.x, mid.y, min.z}, point{min.x, mid.y, mid.z},
        point{mid.x, min.y, min.z}, point{mid.x, min.y, mid.z}, point{mid.x, mid.y, min.z}, mid};
    // maxs are mixed between mid and max
    std::array<point, NumSubBounds> sub_max{mid,
                                            point{mid.x, mid.y, max.z},
                                            point{mid.x, max.y, mid.z},
                                            point{mid.x, max.y, max.z},
                                            point{max.x, mid.y, mid.z},
                                            point{max.x, mid.y, max.z},
                                            point{max.x, max.y, mid.z},
                                            max};
    for (size_t i = 0; i < NumSubBounds; i++) {
        sub_bounds.emplace_back(sub_min[i], sub_max[i]);
        if constexpr (debug::generic) {
            std::cout << "Bound[" << i << "] " << sub_bounds.back() << std::endl;
        }
    }
    return sub_bounds;
}

std::ostream& operator<<(std::ostream& os, Bounds const& b) {
    return os << "Bounds{min=" << b.min << ", max=" << b.max << "}";
}

}  // namespace raytrace
