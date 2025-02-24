#pragma once

#include "raytrace/bounds.hpp"
#include "raytrace/objects/object.hpp"

namespace raytrace {

namespace tree {
class Node {
public:
    /// the number of subnodes is 8, but magic numbers can go awry
    static constexpr size_t NumSubNodes{8U};
    /// Determines the index of the octant that contains the point
    /// @retval 0: (-x, -y, -z)
    /// @retval 1: (-x, -y, +z)
    /// @retval 2: (-x, +y, -z)
    /// @retval 3: (-x, +y, +z)
    /// @retval 4: (+x, -y, -z)
    /// @retval 5: (+x, -y, +z)
    /// @retval 6: (+x, +y, -z)
    /// @retval 7: (+x, +y, +z)
    size_t IndexOf(raytrace::point const& point) const;

    /// Constructs a node with the given bounds
    Node(Bounds const& bounds);

    /// Determines if the object's extent intersects with the bounds of the node
    bool intersects(objects::object const* object) const;

    /// Adds an object to the node if the object extent overlaps with the bounds of the Node
    bool add_object(objects::object const* object);

    /// Returns true if the object is in the existing object list
    /// @param object The object to check for
    bool has(objects::object const* object) const;

    /// Recusively checks to see if the object is under this node
    /// @param object The object to check for
    /// @return true if somewhere under the node (@see has for this node)
    bool under(objects::object const* object) const;

    /// Inserts the object into the list of objects or the subnode lists
    /// @param object The object to insert
    /// @retval true if the object was inserted
    /// @retval false if the object was not inserted
    bool insert(objects::object const* object);

    /// Splits the current bounds into 8 sub-bounds
    std::vector<Bounds> split_bounds();

protected:
    /// The bounding area of the node
    Bounds bounds_;
    /// This node has subnodes to check
    bool has_subnodes_;
    /// The list of the objects in each octant
    std::vector<objects::object const*> objects_;
    /// the subnodes of the node. Always in the order of the octants and nullptr if the octant is empty
    std::vector<Node> nodes_;
};
}  // namespace tree
}  // namespace raytrace