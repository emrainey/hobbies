#include "raytrace/objects/overlap.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

namespace raytrace {
namespace objects {
using namespace linalg::operators;

overlap::overlap(object const& A, object const& B, overlap::type type)
    // FIXME the max collisions are off !
    : object{centroid(A.position(), B.position()),
             (A.max_collisions() + B.max_collisions())/(type == overlap::type::inclusive ? 2 : 1),
             A.is_closed_surface() and B.is_closed_surface()}
    , m_A{A}
    , m_B{B}
    , m_type{type}
    , m_closed_two_hit_surfaces_{m_A.max_collisions() == 2 and m_B.max_collisions() == 2 and m_A.is_closed_surface() and m_B.is_closed_surface()}
    , m_open_two_hit_surfaces_{m_A.max_collisions() == 2 and m_B.max_collisions() == 2 and not m_A.is_closed_surface() and not m_B.is_closed_surface()}
    , m_open_one_hit_surfaces_{m_A.max_collisions() == 1 and m_B.max_collisions() == 1 and not m_A.is_closed_surface() and not m_B.is_closed_surface()}
    {
    // throw_exception_if(m_A.max_collisions() != 2, "First item must have a max of %" PRIu32 " collisions", 2u);
    // throw_exception_if(m_B.max_collisions() != 2, "Second item must have a max of %" PRIu32 " collisions", 2u);
    throw_exception_unless(m_closed_two_hit_surfaces_ or m_open_one_hit_surfaces_ or m_open_two_hit_surfaces_, "Must be one of these %lu types", 3);
}

vector overlap::normal(point const& world_surface_point) const {
    // normals are computed from points where the objects have a collision, this it should be a surface point.
    raytrace::point overlap_point = reverse_transform(world_surface_point);
    vector vA = m_A.normal(overlap_point);
    vector vB = m_B.normal(overlap_point);
    if (vA != R3::null) {
        return forward_transform(vA);
    } else if (vB != R3::null) {
        if (m_type == overlap::type::subtractive) {
            return forward_transform(!vB);
        } else {
            return forward_transform(vB);
        }
    } else {
        return R3::null;
    }
}

hits overlap::collisions_along(ray const& overlap_ray) const {
    /// @note overlap_ray is not in WORLD space, but is in the space of the overlap objects.
    auto object_rayA = m_A.reverse_transform(overlap_ray);
    auto object_rayB = m_B.reverse_transform(overlap_ray);

    hits hitsA = m_A.collisions_along(object_rayA);
    hits hitsB = m_B.collisions_along(object_rayB);

    // early exit
    if (hitsA.size() == 0 and hitsB.size() == 0) {
        return hits();  // empty
    }

    auto sorter = [](precision a, precision b) -> bool { return (a < b); };

    // sort them individually so we can make some logical deductions later.
    std::sort(hitsA.begin(), hitsA.end(), sorter);
    std::sort(hitsB.begin(), hitsB.end(), sorter);
    // merge the lists, then sort distances.
    hits hitsAB;
    hitsAB.insert(hitsAB.end(), hitsA.begin(), hitsA.end());
    hitsAB.insert(hitsAB.end(), hitsB.begin(), hitsB.end());
    std::sort(hitsAB.begin(), hitsAB.end(), sorter);

    if (m_type == overlap::type::additive) {
        // remove the inner alternating hit from the hits lists
        // if just [A0, A1] or [B0, B1] then return that
        if (hitsA.size() == 0) {
            return hitsB;
        }
        if (hitsB.size() == 0) {
            return hitsA;
        }
        if (m_closed_two_hit_surfaces_) {
            // if [A0, A1, B0, B1] then return all
            // if [B0, B1, A0, A1] then return all
            if (std::equal(hitsAB.begin(), hitsAB.begin() + hitsA.size(), hitsA.begin())
                or std::equal(hitsAB.begin(), hitsAB.begin() + hitsB.size(), hitsB.begin())) {
                // if it starts with only hitsA or only hitsB
                return hitsAB;
            }
            // if [A0, B0, A1, B1] then return [A0, B1]
            // if [B0, A0, B1, A1] then return [B0, A1]
            if ((hitsAB[0] == hitsA[0] and hitsAB[1] == hitsB[0])
                or (hitsAB[0] == hitsB[0] and hitsAB[1] == hitsA[0])) {
                // remove second and third (end is exclusive)
                hitsAB.erase(hitsAB.begin() + 1, hitsAB.end());
                return hitsAB;
            }
        }
        // if [A0, B0, B1, A0] them remove [B0, B1] and return [A0, A1] or reverse ?
    } else if (m_type == overlap::type::subtractive) {
        // B is the subtractive object. negate B's normals (done in the normal function)
        // if just [B0, B1] with no A, return empty
        if (hitsA.size() == 0) {
            return hits();  // empty;
        }
        // if just [A0, A1] with no B hits, return that.
        if (hitsB.size() == 0) {
            return hitsA;  // should just have A items...
        }

        if (m_closed_two_hit_surfaces_) {
            // if [A0, A1, B0, B1] then return [A0, A1]
            if (std::equal(hitsAB.begin(), hitsAB.begin() + hitsA.size(), hitsA.begin())) {
                return hitsA;
            }
            // if [B0, B1, A0, A1] then return A
            // if starts with hitsB
            if (std::equal(hitsAB.begin(), hitsAB.begin() + hitsB.size(), hitsB.begin())) {
                return hitsA;
            }
            // if [B0, A0, A1, B1] then return empty
            if (hitsAB[0] == hitsB[0] and hitsAB[3] == hitsB[1]) {
                return hits();  // empty
            }
            // if [B0, A0, B1, A1] then return [B1, A1]
            if (hitsAB[0] == hitsB[0] and hitsAB[1] == hitsA[0]) {
                return hits(hitsAB.begin() + 2, hitsAB.end());
            }
            // if [A0, B0, B1, A1] then return all
            // if [A0, B0, A1, B1] then return [A0, B0]
            if (hitsAB[0] == hitsA[0] and hitsAB[1] == hitsB[0]) {
                if (hitsAB[2] == hitsA[1]) {
                    return hits(hitsAB.begin(), hitsAB.begin() + 2);
                } else {
                    return hitsAB;
                }
            }
        }
    } else if (m_type == overlap::type::inclusive) {
        // return inner hitsAB
        if (m_closed_two_hit_surfaces_) {
            // if just [A0, A1] or [B0, B1] then return empty.
            if (hitsA.size() == 0 or hitsB.size() == 0) {
                return hits();  // empty
            }
            // if [A0, A1, B0, B1] return empty
            // if [B0, B1, A0, A1] return empty
            if (std::equal(hitsAB.begin(), hitsAB.begin() + hitsA.size(), hitsA.begin())
                or std::equal(hitsAB.begin(), hitsAB.begin() + hitsB.size(), hitsB.begin())) {
                // if it starts with only hitsA or only hitsB
                return hits();  // empty
            }
            // if [A0, B0, A1, B1] then return [B0, A1]
            // if [B0, A0, B1, A1] then return [A0, B1]
            // if [A0, B0, B1, A1] then return [B0, B1]
            // if [B0, A0, A1, B1] then return [A0, A1]
            // if the first and second elements are from different objects...
            if ((hitsAB[0] == hitsA[0] and hitsAB[1] == hitsB[0])
                or (hitsAB[0] == hitsB[0] and hitsAB[1] == hitsA[0])) {
                return hits(hitsAB.begin() + 1, hitsAB.end() - 1);
            }
        }
        if (m_open_two_hit_surfaces_) {
            // if just [A0, A1] or [B0, B1] then check to see if those points are inside the other
            if (hitsA.size() == 0) { // then B has to have hits
                point const B0 = object_rayB.distance_along(hitsB[0]);
                point const B1 = object_rayB.distance_along(hitsB[1]);
                if (not m_A.is_outside(B0) and not m_A.is_outside(B1)) {
                    return hitsB;
                }
                return hits{};
            }
            if (hitsB.size() == 0) { // then A has to have hits
                point const A0 = object_rayA.distance_along(hitsA[0]);
                point const A1 = object_rayA.distance_along(hitsA[1]);
                if (not m_B.is_outside(A0) and not m_B.is_outside(A1)) {
                    return hitsA;
                }
                return hits{};
            }
            // if [A0, B0, A1, B1] then return [B0, A1]
            // if [B0, A0, B1, A1] then return [A0, B1]
            // if [A0, B0, B1, A1] then return [B0, B1]
            // if [B0, A0, A1, B1] then return [A0, A1]
            // if the first and second elements are from different objects...
            if ((hitsAB[0] == hitsA[0] and hitsAB[1] == hitsB[0])
                or (hitsAB[0] == hitsB[0] and hitsAB[1] == hitsA[0])) {
                return hits(hitsAB.begin() + 1, hitsAB.end() - 1);
            }
        }
    } else if (m_type == overlap::type::exclusive) {
        // if just [A0, A1] or [B0, B1] then return that
        if (hitsA.size() == 0) {
            return hitsB;
        }
        if (hitsB.size() == 0) {
            return hitsA;
        }
    }
    return hits();  // empty
}

bool overlap::is_surface_point(point const& world_point) const {
    // based on which type determine how they overlap
    return m_A.is_surface_point(world_point) or m_B.is_surface_point(world_point);
}

bool overlap::is_outside(point const& world_point) const {
    point const overlap_point = forward_transform(world_point);
    bool const outside_A = m_A.is_outside(overlap_point);
    bool const outside_B = m_B.is_outside(overlap_point);
    if (m_type == overlap::type::additive) {
        return outside_A and outside_B;
    } else if (m_type == overlap::type::subtractive) {
        return not (not outside_A and outside_B);
    } else if (m_type == overlap::type::inclusive) {
        return outside_A or outside_B;
    } else if (m_type == overlap::type::exclusive) {
        return not (outside_A xor outside_B);
    }
    return false;
}

bool overlap::is_along_infinite_extent(ray const& world_ray) const {
    return m_A.is_along_infinite_extent(world_ray) and m_B.is_along_infinite_extent(world_ray);
}

image::point overlap::map(point const& object_surface_point __attribute__((unused))) const {
    image::point uv{0, 0};
    // @TODO check if it's a surface point for the objects then call their map.
    return uv;
}

void overlap::print(char const str[]) const {
    m_A.print(str);
    m_B.print(str);
}

precision overlap::get_object_extent(void) const {
    // we want this to be additive as this is for collision checking.
    return std::max(m_A.get_object_extent(), m_B.get_object_extent());
}

}  // namespace objects
}  // namespace raytrace
