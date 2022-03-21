#include "doorgame/parameters.hpp"

namespace doorgame {

std::ostream& operator<<(std::ostream& os, Parameter p) {
    if (std::holds_alternative<std::monostate>(p)) {
        os << "(empty)";
    } else if (std::holds_alternative<Direction>(p)) {
        Direction d = std::get<1>(p);
        os << d;
    } else if (std::holds_alternative<Item>(p)) {
        Item i = std::get<2>(p);
        os << i;
    } else if (std::holds_alternative<Target>(p)) {
        Target t = std::get<3>(p);
        os << t;
    }
    return os;
}

}