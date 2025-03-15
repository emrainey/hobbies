#pragma once
/// @file
/// The debuggable class definition
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>

namespace basal {

/// The bitfield definition of all known zones.
union zone_mask {
    constexpr zone_mask() : all{0} {
    }
    uint32_t all;  ///< the union of all zones below
    /// This struct of bits should follow the enumeration above.
    struct {
        uint32_t priority : 1;
        uint32_t fatal : 1;
        uint32_t error : 1;
        uint32_t warn : 1;
        uint32_t api : 1;
        uint32_t info : 1;
        uint32_t : 26;  ///< NOTE when a new zone is added one should be removed from here
    } zones;
};

class debuggable {
protected:
    //! These are the allowed zone masks (can even be changed in const methods)
    mutable zone_mask m_mask;

    /// Default Constructor
    constexpr debuggable() : m_mask{} {
    }

    /// Protected Constructor
    constexpr debuggable(zone_mask zm) : m_mask{zm} {
    }

    /// Protected Destructor
    virtual ~debuggable() = default;

    /// Method Tracing Template (instead of a macro)
    template <typename ReturnType, typename... ParameterTypes>
    ReturnType trace(std::function<ReturnType(ParameterTypes...)> method, ParameterTypes... args) {
        if (mask().zones.api) {
            emit("+%s\n", __FUNCTION__);
        }
        ReturnType ret = method(&args...);
        if (mask().zones.api) {
            emit("-%s\n", __FUNCTION__);
        }
        return ret;  // this will require that ReturnType isCopyable
    }

    static constexpr size_t local_stack_for_print = 250;

    virtual void emit(char const fmt[], ...) const;

public:
    inline zone_mask& mask() noexcept {
        return m_mask;
    }

    inline zone_mask const& mask() const noexcept {
        return m_mask;
    }
};

}  // namespace basal
