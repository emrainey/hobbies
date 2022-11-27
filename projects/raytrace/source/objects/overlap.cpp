#include "raytrace/objects/overlap.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

namespace raytrace {
namespace objects {
using namespace linalg::operators;

overlap::overlap(const object& A, const object& B, overlap::type type) : m_A{A}, m_B{B}, m_type{type} {
}

vector overlap::normal(const point& world_surface_point) const {
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

hits overlap::collisions_along(const ray& overlap_ray) const {
    /// @note are we going to assume that each of these has zero or two points?
    // @warning overlap_ray is not in WORLD space, but is in the space of the overlap objects.

    hits hitsA = m_A.collisions_along(m_A.reverse_transform(overlap_ray));
    hits hitsB = m_B.collisions_along(m_B.reverse_transform(overlap_ray));

    // early exit
    if (hitsA.size() == 0 and hitsB.size() == 0) {
        return hits();  // empty
    }

    auto sorter = [](element_type a, element_type b) -> bool { return (a < b); };

    // sort them individually so we can make some logical deductions later.
    std::sort(hitsA.begin(), hitsA.end(), sorter);
    std::sort(hitsB.begin(), hitsB.end(), sorter);
    // merge the lists, then sort distances.
    hits hitsAB;
    hitsAB.insert(hitsAB.end(), hitsA.begin(), hitsA.end());
    hitsAB.insert(hitsAB.end(), hitsB.begin(), hitsB.end());
    std::sort(hitsAB.begin(), hitsAB.end(), sorter);

    // classify the type of colliders here
    bool two_closed_simple_surfaces = (m_A.max_collisions() == 2 and m_B.max_collisions() == 2
                                       and m_A.is_closed_surface() and m_B.is_closed_surface());

    if (m_type == overlap::type::additive) {
        // remove the inner alternating hit from the hits lists
        // if just [A0, A1] or [B0, B1] then return that
        if (hitsA.size() == 0) {
            return hitsB;
        }
        if (hitsB.size() == 0) {
            return hitsA;
        }
        if (two_closed_simple_surfaces) {
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

        if (two_closed_simple_surfaces) {
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
        // if just [A0, A1] or [B0, B1] then return empty.
        if (hitsA.size() == 0 or hitsB.size() == 0) {
            return hits();  // empty
        }
        if (two_closed_simple_surfaces) {
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
            // if the first and second elements are from differnet objects...
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

bool overlap::is_surface_point(const point& world_point) const {
    // based on which type determine how they overlap
    return m_A.is_surface_point(world_point) or m_B.is_surface_point(world_point);
}

image::point overlap::map(const point& object_surface_point __attribute__((unused))) const {
    image::point uv{0, 0};
    return uv;
}

void overlap::print(const char str[]) const {
    m_A.print(str);
    m_B.print(str);
}

element_type overlap::get_object_extant(void) const {
    // we want this to be additive as this is for collision checking.
    return std::max(m_A.get_object_extant(), m_B.get_object_extant());
}

}  // namespace objects
}  // namespace raytrace
