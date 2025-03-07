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

size_t Node::object_count() const {
    return objects_.size();
}

size_t Node::node_count() const {
    return nodes_.size();
}

bool Node::intersects(objects::object const* object) const {
    return bounds_.intersects(object->get_world_bounds());
}

objects::object::hits Node::intersects(raytrace::ray const& ray) const {
    hits hits;
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

bool Node::insert(objects::object const* object) {
    size_t added{0U};
    size_t intersects{0U};
    // scan all the subnodes for where to put the object
    for (size_t i = 0; i < NumSubNodes; i++) {
        if (nodes_[i].intersects(object)) {
            intersects++;
        }
    }
    if (intersects == 8) {
        // if the object intersects all the subnodes, then it stays in this node
        objects_.push_back(object);
        added++;
    } else {
        // if the object intersects some of the subnodes, then it goes into the subnode
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
                std::vector<Bounds> sub_bounds = split_bounds();
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

std::vector<Bounds> Node::split_bounds() {
    std::vector<Bounds> sub_bounds;
    point mid = bounds_.center();
    point min = bounds_.min;
    point max = bounds_.max;
    // need to create all the min points and max points
    // mins are mixed between min and mid
    std::array<point, NumSubNodes> sub_min{
        point{min.x, min.y, min.z}, point{min.x, min.y, mid.z}, point{min.x, mid.y, min.z}, point{min.x, mid.y, mid.z},
        point{mid.x, min.y, min.z}, point{mid.x, min.y, mid.z}, point{mid.x, mid.y, min.z}, mid};
    // maxs are mixed between mid and max
    std::array<point, NumSubNodes> sub_max{mid,
                                           point{mid.x, mid.y, max.z},
                                           point{mid.x, max.y, mid.z},
                                           point{mid.x, max.y, max.z},
                                           point{max.x, mid.y, mid.z},
                                           point{max.x, mid.y, max.z},
                                           point{max.x, max.y, mid.z},
                                           max};
    for (size_t i = 0; i < NumSubNodes; i++) {
        sub_bounds.emplace_back(sub_min[i], sub_max[i]);
        std::cout << "Bound[" << i << "] Min " << sub_min[i] << " Max " << sub_max[i] << std::endl;
    }
    return sub_bounds;
}

}  // namespace tree
}  // namespace raytrace