#pragma once

#include <variant>
#include <vector>

#include "basal/basal.hpp"

namespace doorgame {

/// A list of boolean attributes
enum class Attribute : uint16_t {
    Investigated = 0,  //!< Any Target can be investigated

    MAX,  //!< The maximum index of all attributes which we use to determine the
};

/// An extensible set of bool attributes in an object
class HasAttributes {
public:
    /// Retrieves an attribute
    bool get(Attribute attr) const;

    /// Sets the state of an attribute
    void set(Attribute attr, bool value);

protected:
    std::vector<Attribute::MAX> attributes;
};

}  // namespace doorgame