#pragma once

#include "raytrace/bounds.hpp"
#include "raytrace/objects/object.hpp"

namespace raytrace {

namespace tree {

/// @brief  A node in the tree structure. It contains a bounding box and a list of objects. Each node can have up to N
/// subnodes. Abstractly a node is a bounding box that can contain or share objects with other peer-level nodes. When an
/// object is shared it should not be add to the subnodes of the node. The node can be split into 8 subnodes (octants)
/// if it has more than a certain number of objects. The node can also be used to determine if a ray intersects with the
/// objects in the node or any of its subnodes.
/// @note   The node is not a tree, but it can be used to build a tree structure by linking nodes together.
class Node {
public:
    using hits = objects::object::hits;
    /// the number of subnodes is 8, but magic numbers can go awry
    static constexpr size_t NumSubNodes{Bounds::NumSubBounds};
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

    /// @return the bounds of the node
    inline Bounds const& bounds() const {
        return bounds_;
    }

    /// Determines the hits of the ray where it intersects with the objects in the node or it's sub-nodes
    /// @retval No hits if the ray does not intersect with anything in the node tree.
    hits intersects(raytrace::ray const& ray) const;

    /// Determines if the object's extent intersects with the bounds of the node
    bool intersects(objects::object const* object) const;

    /// Adds an object to the node if the object extent overlaps with the bounds of the Node
    bool add_object(objects::object const* object);

    /// Iterates over all the subnodes in the node
    bool any_of(std::function<bool(Node const&)> const& func) const;

    /// Returns true if the object is in this nodes object list
    /// @note This does not check the subnodes
    /// @param object The object to check for
    bool has(objects::object const* object) const;

    /// Recursively checks to see if the object is in any of this node's subnodes.
    /// @note Does not check this node's object list.
    /// @param object The object to check for
    /// @return true if somewhere under the node (@see has for this node)
    bool under(objects::object const* object) const;

    /// Checks if the object is in this node or any of its subnodes
    bool contains(objects::object const* object) const;

    /// @return the number of objects in this node only, not subnodes
    size_t direct_object_count() const;

    /// @return the number of sub nodes in this node
    size_t node_count() const;

    /// @return the number of objects in this node and all subnodes
    size_t all_object_count() const;

    /// Inserts the object into the list of objects or the subnode lists
    /// @param object The object to insert
    /// @retval true if the object was inserted
    /// @retval false if the object was not inserted
    bool insert(objects::object const* object);

    /// Returns the number of instances of the object that are in this node tree.
    size_t count_of(objects::object const* object) const;

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