#include "raytrace/tree.hpp"

namespace raytrace {

namespace tree {

/// Constructs a node with the given bounds
Node::Node(Bounds const& bounds) : bounds_(bounds), has_subnodes_{false}, objects_{}, nodes_{} {
}

size_t Node::IndexOf(raytrace::point const& point) const {
    size_t index = 0;
    raytrace::point const& middle = bounds_.center();
    if (point.x > middle.x) {
        index |= 4;
    }
    if (point.y > middle.y) {
        index |= 2;
    }
    if (point.z > middle.z) {
        index |= 1;
    }
    return index;
}

bool Node::has(objects::object const* object) const {
    return std::find(objects_.begin(), objects_.end(), object) != objects_.end();
}

bool Node::under(objects::object const* object) const {
    // check each subnode for the object
    for (auto const& node : nodes_) {
        if (node.has(object)) {
            return true;
        } else {
            if (node.under(object)) {
                return true;
            }
        }
    }
    return false;
}

bool Node::contains(objects::object const* object) const {
    return has(object) || under(object);
}

size_t Node::direct_object_count() const {
    return objects_.size();
}

size_t Node::node_count() const {
    return nodes_.size();
}

size_t Node::all_object_count() const {
    size_t count = direct_object_count();
    for (auto const& node : nodes_) {
        count += node.all_object_count();
    }
    return count;
}

bool Node::intersects(objects::object const* object) const {
    return bounds_.intersects(object->get_world_bounds());
}

objects::object::hits Node::intersects(raytrace::ray const& ray) const {
    hits hits;
    // if there's no objects and no subnodes, then it doesn't matter if it intersects?
    // we still want to know if it could have intersected. So leave it this way.
    if (bounds_.intersects(ray)) {
        for (auto const& object : objects_) {
            auto hit = object->intersect(ray);
            if (get_type(hit.intersect) == IntersectionType::Point) {
                hits.push_back(hit);
            }
        }
        for (auto const& subnode : nodes_) {
            if (subnode.bounds().intersects(ray)) {
                auto subhits = subnode.intersects(ray);
                hits.insert(hits.end(), subhits.begin(), subhits.end());
            }
        }
    }
    return hits;
}

bool Node::any_of(std::function<bool(Node const&)> const& func) const {
    std::vector<bool> ret(nodes_.size());  // all false at first
    for (auto& node : nodes_) {
        ret.push_back(func(node));
    }
    return std::any_of(ret.begin(), ret.end(), [](bool b) { return b; });
}

size_t Node::count_of(objects::object const* object) const {
    size_t count = 0;
    if (has(object)) {
        count++;
    }
    for (auto const& node : nodes_) {
        count += node.count_of(object);
    }
    return count;
}

bool Node::insert(objects::object const* object) {
    size_t added{0U};
    size_t intersects{0U};
    // scan all the subnodes for where to put the object
    for (size_t i = 0; i < NumSubNodes; i++) {
        if (nodes_[i].intersects(object)) {
            intersects++;
        }
    }
    if (intersects > 1) {
        // if the object intersects more than one of the subnodes, then it stays in this node
        objects_.push_back(object);
        added++;
    } else {
        // if the object intersects only a sub node then it goes into the subnode
        for (size_t i = 0; i < NumSubNodes; i++) {
            if (nodes_[i].add_object(object)) {
                added++;
            }
        }
    }
    return (added > 0U);
}

bool Node::add_object(objects::object const* object) {
    size_t added = 0;
    if (intersects(object)) {
        if (not has_subnodes_) {
            objects_.push_back(object);
            added++;
            if (objects_.size() > NumSubNodes) {
                // mark the node as now having subnodes
                has_subnodes_ = true;
                // first create 8 sub-bounds from the current bounds
                std::vector<Bounds> sub_bounds = bounds_.split();
                for (size_t i = 0; i < NumSubNodes; i++) {
                    nodes_.push_back(sub_bounds[i]);
                }
                // copy the object vector here
                std::vector<objects::object const*> temp_objects = objects_;
                objects_.clear();
                /// move all objects into the sub-nodes if they intersect, any object that intersects all the subnodes
                /// stays in this node.
                for (auto obj : temp_objects) {
                    if (insert(obj)) {
                        // do nothing
                    }
                }
            }
        } else {
            if (insert(object)) {
                added++;
            }
        }
    }
    return (added > 0U);
}

std::ostream& operator<<(std::ostream& os, Node const& node) {
    os << "Node: " << node.bounds_ << " has_subnodes: " << node.has_subnodes_ << " objects: " << node.objects_.size()
       << " nodes: " << node.nodes_.size();
    return os;
}

}  // namespace tree
}  // namespace raytrace