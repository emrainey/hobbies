#pragma once

#include "raytrace/raytrace.hpp"

namespace raytrace {

class group {
public:
    group() : m_objects{} {
    }

    virtual ~group() = default;

    inline void move_by(raytrace::vector const& vec) {
        for (auto& obj : m_objects) {
            obj->move_by(vec);
        }
    }

    inline void add_to_scene(scene& sc) {
        for (auto& obj : m_objects) {
            sc.add_object(obj);
        }
    }

protected:
    std::vector<objects::object*> m_objects;
};

}  // namespace raytrace