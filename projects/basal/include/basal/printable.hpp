#pragma once

#include <inttypes.h>

#include <iostream>
#include <string>

#ifndef PRIz
#if defined(__x86_64__) or defined(__arm__) or defined(__aarch64__)
#define PRIz "zu"
#endif
#endif

namespace basal {

/// A base class used to enforce a printing mechanism
class printable {
public:
    /// Character Array printing mechanism
    virtual void print(char const[]) const = 0;

    /// String printing mechanism.
    inline void print(std::string str) const {
        print(str.c_str());
    }

protected:
    /// Virtual deconstructor
    ~printable() = default;
};

}  // namespace basal
